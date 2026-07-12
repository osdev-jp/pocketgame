#include "parser.h"
#include <stdio.h>
#include <string.h>

static bool token_equals(const Token *token, const char *text) {
	size_t length;

	length = strlen(text);

	if (token->length != length) { return false; }

	return memcmp(token->start, text, length) == 0;
}

static bool parser_is_directive(const Token *token) {
	static const char *names[] = {"GAME",	 "GRID",     "CURSOR",
				      "STATE",	 "INPUT",    "FLAG",
				      "MESSAGE", "ON_START", "ON_FRAME"};
	size_t i;

	if (token->type != TOKEN_IDENTIFIER) { return false; }

	for (i = 0; i < sizeof(names) / sizeof(names[0]); i++) {
		if (token_equals(token, names[i])) { return true; }
	}

	return false;
}

static void
parser_error_at(Parser *parser, const Token *token, const char *message) {
	fprintf(stderr, "%s:%zu:%zu: %s", parser->input_path, token->line,
		token->column, message);

	if (token->type != TOKEN_EOF) {
		fprintf(stderr, " near '%.*s'", (int)token->length,
			token->start);
	}

	fputc('\n', stderr);
	parser->had_error = true;
}

static void parser_advance(Parser *parser) {
	parser->previous = parser->current;

	for (;;) {
		parser->current = lexer_next_token(&parser->lexer);

		if (parser->current.type != TOKEN_INVALID) { return; }

		parser_error_at(parser, &parser->current, "invalid token");
	}
}

static bool parser_check(const Parser *parser, TokenType type) {
	return parser->current.type == type;
}

static bool parser_match(Parser *parser, TokenType type) {
	if (!parser_check(parser, type)) { return false; }

	parser_advance(parser);
	return true;
}

static bool
parser_consume(Parser *parser, TokenType type, const char *message) {
	if (!parser_check(parser, type)) {
		parser_error_at(parser, &parser->current, message);
		return false;
	}

	parser_advance(parser);
	return true;
}

static bool parser_skip_balanced(Parser *parser,
				 TokenType open_type,
				 TokenType close_type,
				 const char *open_message,
				 const char *close_message) {
	size_t depth;

	if (!parser_consume(parser, open_type, open_message)) { return false; }

	depth = 1;

	while (depth > 0) {
		if (parser_check(parser, TOKEN_EOF)) {
			parser_error_at(parser, &parser->current,
					close_message);
			return false;
		}

		if (parser_match(parser, open_type)) {
			depth++;
			continue;
		}

		if (parser_match(parser, close_type)) {
			depth--;
			continue;
		}

		parser_advance(parser);
	}

	return true;
}

static bool parser_parse_directive(Parser *parser, Declaration *declaration) {
	declaration->type = DECLARATION_DIRECTIVE;
	declaration->name = parser->current;
	declaration->line = parser->current.line;

	parser_advance(parser);

	if (!parser_skip_balanced(parser, TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN,
				  "expected '(' after directive name",
				  "expected ')' after directive arguments")) {
		return false;
	}

	if (!parser_consume(parser, TOKEN_SEMICOLON,
			    "expected ';' after directive")) {
		return false;
	}

	return true;
}

static bool parser_parse_function(Parser *parser, Declaration *declaration) {
	Token return_type;
	Token name;

	return_type = parser->current;
	parser_advance(parser);

	if (!parser_check(parser, TOKEN_IDENTIFIER)) {
		parser_error_at(parser, &parser->current,
				"expected function name");
		return false;
	}

	name = parser->current;
	parser_advance(parser);

	if (!parser_skip_balanced(parser, TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN,
				  "expected '(' after function name",
				  "expected ')' after function parameters")) {
		return false;
	}

	declaration->name = name;
	declaration->return_type = return_type;
	declaration->line = return_type.line;

	if (parser_match(parser, TOKEN_SEMICOLON)) {
		declaration->type = DECLARATION_FUNCTION_PROTOTYPE;
		return true;
	}

	if (parser_check(parser, TOKEN_LEFT_BRACE)) {
		declaration->type = DECLARATION_FUNCTION_DEFINITION;

		return parser_skip_balanced(parser, TOKEN_LEFT_BRACE,
					    TOKEN_RIGHT_BRACE,
					    "expected '{' before function body",
					    "expected '}' after function body");
	}

	parser_error_at(parser, &parser->current,
			"expected ';' or function body");

	return false;
}

void parser_init(Parser *parser, const char *input_path, const char *source) {
	lexer_init(&parser->lexer, source);

	parser->current.type = TOKEN_EOF;
	parser->current.start = source;
	parser->current.length = 0;
	parser->current.line = 1;
	parser->current.column = 1;
	parser->previous = parser->current;
	parser->input_path = input_path;
	parser->had_error = false;

	parser_advance(parser);
}

bool parser_next_declaration(Parser *parser, Declaration *declaration) {
	memset(declaration, 0, sizeof(*declaration));
	declaration->return_type.type = TOKEN_EOF;

	if (parser_check(parser, TOKEN_EOF)) { return false; }

	if (parser_is_directive(&parser->current)) {
		return parser_parse_directive(parser, declaration);
	}

	if (parser_check(parser, TOKEN_IDENTIFIER)) {
		return parser_parse_function(parser, declaration);
	}

	parser_error_at(parser, &parser->current,
			"expected directive or function declaration");

	return false;
}