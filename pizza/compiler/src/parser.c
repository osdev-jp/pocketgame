#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	Token *items;
	size_t count;
	size_t capacity;
} TokenList;

static bool parser_parse_statement(Parser *parser, Statement **statement);
static bool parser_parse_block_statement(Parser *parser, Statement **statement);

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
	declaration->body = NULL;

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
	Statement *body;
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
	declaration->body = NULL;

	if (parser_match(parser, TOKEN_SEMICOLON)) {
		declaration->type = DECLARATION_FUNCTION_PROTOTYPE;
		return true;
	}

	if (parser_check(parser, TOKEN_LEFT_BRACE)) {
		body = NULL;

		if (!parser_parse_block_statement(parser, &body)) {
			return false;
		}

		declaration->type = DECLARATION_FUNCTION_DEFINITION;
		declaration->body = body;
		return true;
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

static void token_list_init(TokenList *list) {
	list->items = NULL;
	list->count = 0;
	list->capacity = 0;
}

static void token_list_destroy(TokenList *list) {
	free(list->items);
	token_list_init(list);
}

static bool token_list_append(Parser *parser, TokenList *list, Token token) {
	Token *items;
	size_t capacity;

	if (list->count == list->capacity) {
		capacity = list->capacity == 0 ? 8 : list->capacity * 2;

		items = realloc(list->items, capacity * sizeof(*list->items));

		if (items == NULL) {
			parser_error_at(parser, &parser->current,
					"failed to alloc token list");
			return false;
		}

		list->items = items;
		list->capacity = capacity;
	}

	list->items[list->count] = token;
	list->count++;

	return true;
}

static Token *token_list_take(TokenList *list, size_t *count) {
	Token *items;

	items = list->items;
	*count = list->count;

	token_list_init(list);
	return items;
}

static bool parser_collect_parenthesized_tokens(Parser *parser,
						Token **tokens,
						size_t *token_count) {
	TokenList list;
	size_t depth;

	token_list_init(&list);

	if (!parser_consume(parser, TOKEN_LEFT_PAREN, "expected '('")) {
		return false;
	}

	depth = 1;

	while (depth > 0) {
		if (parser_check(parser, TOKEN_EOF)) {
			parser_error_at(parser, &parser->current,
					"expected ')'");

			token_list_destroy(&list);
			return false;
		}

		if (parser_check(parser, TOKEN_LEFT_PAREN)) {
			depth++;
		} else if (parser_check(parser, TOKEN_RIGHT_PAREN)) {
			depth--;

			if (depth == 0) {
				parser_advance(parser);
				break;
			}
		}

		if (!token_list_append(parser, &list, parser->current)) {
			token_list_destroy(&list);
			return false;
		}

		parser_advance(parser);
	}

	*tokens = token_list_take(&list, token_count);
	return true;
}

static bool parser_collect_until_semicolon(Parser *parser,
					   Token **tokens,
					   size_t *token_count) {
	TokenList list;
	size_t parenthesis_depth;
	size_t bracket_depth;

	token_list_init(&list);
	parenthesis_depth = 0;
	bracket_depth = 0;

	while (!parser_check(parser, TOKEN_EOF)) {
		if (parser_check(parser, TOKEN_SEMICOLON) &&
		    parenthesis_depth == 0 && bracket_depth == 0) {
			parser_advance(parser);

			*tokens = token_list_take(&list, token_count);
			return true;
		}

		if (parser_check(parser, TOKEN_LEFT_BRACE) ||
		    parser_check(parser, TOKEN_RIGHT_BRACE)) {
			parser_error_at(parser, &parser->current,
					"expected ';' before block boundary");

			token_list_destroy(&list);
			return false;
		}

		if (parser_check(parser, TOKEN_LEFT_PAREN)) {
			parenthesis_depth++;
		} else if (parser_check(parser, TOKEN_RIGHT_PAREN)) {
			if (parenthesis_depth == 0) {
				parser_error_at(parser, &parser->current,
						"unexpected ')'");

				token_list_destroy(&list);
				return false;
			}

			parenthesis_depth--;
		} else if (parser_check(parser, TOKEN_LEFT_BRACKET)) {
			bracket_depth++;
		} else if (parser_check(parser, TOKEN_RIGHT_BRACKET)) {
			if (bracket_depth == 0) {
				parser_error_at(parser, &parser->current,
						"unexpected ']'");

				token_list_destroy(&list);
				return false;
			}

			bracket_depth--;
		}

		if (!token_list_append(parser, &list, parser->current)) {
			token_list_destroy(&list);
			return false;
		}

		parser_advance(parser);
	}

	parser_error_at(parser, &parser->current, "expected ';'");

	token_list_destroy(&list);
	return false;
}

static bool parser_parse_block_statement(Parser *parser,
					 Statement **statement) {
	Statement *block;
	Statement *child;

	block = statement_create(STATEMENT_BLOCK, parser->current.line);

	if (block == NULL) {
		parser_error_at(parser, &parser->current,
				"failed to allocate block statement");

		return false;
	}

	if (!parser_consume(parser, TOKEN_LEFT_BRACE, "expected '{'")) {
		statement_destroy(block);
		return false;
	}

	while (!parser_check(parser, TOKEN_RIGHT_BRACE) &&
	       !parser_check(parser, TOKEN_EOF)) {
		child = NULL;

		if (!parser_parse_statement(parser, &child)) {
			statement_destroy(block);
			return false;
		}

		if (statement_list_append(&block->block, child) != 0) {
			parser_error_at(parser, &parser->current,
					"failed to append statement");

			statement_destroy(child);
			statement_destroy(block);
			return false;
		}
	}

	if (!parser_consume(parser, TOKEN_RIGHT_BRACE,
			    "expected '}' after block")) {
		statement_destroy(block);
		return false;
	}

	*statement = block;
	return true;
}

static bool parser_parse_if_statement(Parser *parser, Statement **statement) {
	Statement *if_statement;
	Statement *then_branch;
	Token *condition_tokens;
	size_t condition_token_count;
	size_t line;

	line = parser->current.line;
	parser_advance(parser);

	condition_tokens = NULL;
	condition_token_count = 0;

	if (!parser_collect_parenthesized_tokens(parser, &condition_tokens,
						 &condition_token_count)) {
		return false;
	}

	then_branch = NULL;

	if (!parser_parse_statement(parser, &then_branch)) {
		free(condition_tokens);
		return false;
	}

	if_statement = statement_create(STATEMENT_IF, line);

	if (if_statement == NULL) {
		parser_error_at(parser, &parser->current,
				"failed to allocate if statement");

		free(condition_tokens);
		statement_destroy(then_branch);
		return false;
	}

	if_statement->if_statement.tokens = condition_tokens;
	if_statement->if_statement.token_count = condition_token_count;
	if_statement->if_statement.branch = then_branch;

	*statement = if_statement;
	return true;
}

static bool parser_parse_while_statement(Parser *parser,
					 Statement **statement) {
	Statement *while_statement;
	Statement *branch;
	Token *tokens;
	size_t token_count;
	size_t line;

	line = parser->current.line;
	parser_advance(parser);

	tokens = NULL;
	token_count = 0;

	if (!parser_collect_parenthesized_tokens(parser, &tokens,
						 &token_count)) {
		return false;
	}

	branch = NULL;

	if (!parser_parse_statement(parser, &branch)) {
		free(tokens);
		return false;
	}

	while_statement = statement_create(STATEMENT_WHILE, line);

	if (while_statement == NULL) {
		parser_error_at(parser, &parser->current,
				"failed to allocate while statement");

		free(tokens);
		statement_destroy(branch);
		return false;
	}

	while_statement->while_statement.tokens = tokens;
	while_statement->while_statement.token_count = token_count;
	while_statement->while_statement.branch = branch;

	*statement = while_statement;
	return true;
}

static bool parser_parse_return_statement(Parser *parser,
					  Statement **statement) {
	Statement *return_statement;
	Token *tokens;
	size_t token_count;
	size_t line;

	line = parser->current.line;
	parser_advance(parser);

	tokens = NULL;
	token_count = 0;

	if (!parser_collect_until_semicolon(parser, &tokens, &token_count)) {
		return false;
	}

	return_statement = statement_create(STATEMENT_RETURN, line);

	if (return_statement == NULL) {
		parser_error_at(parser, &parser->current,
				"failed to allocate return statement");

		free(tokens);
		return false;
	}

	return_statement->return_statement.tokens = tokens;
	return_statement->return_statement.token_count = token_count;

	*statement = return_statement;
	return true;
}

static bool parser_parse_expression_statement(Parser *parser,
					      Statement **statement) {
	Statement *expression_statement;
	Token *tokens;
	size_t token_count;
	size_t line;

	line = parser->current.line;
	tokens = NULL;
	token_count = 0;

	if (!parser_collect_until_semicolon(parser, &tokens, &token_count)) {
		return false;
	}

	expression_statement = statement_create(STATEMENT_EXPRESSION, line);

	if (expression_statement == NULL) {
		parser_error_at(parser, &parser->current,
				"failed to allocate expression statement");

		free(tokens);
		return false;
	}

	expression_statement->expression.tokens = tokens;
	expression_statement->expression.token_count = token_count;

	*statement = expression_statement;
	return true;
}

static bool parser_parse_statement(Parser *parser, Statement **statement) {
	if (parser_check(parser, TOKEN_LEFT_BRACE)) {
		return parser_parse_block_statement(parser, statement);
	}

	if (parser_check(parser, TOKEN_IDENTIFIER) &&
	    token_equals(&parser->current, "if")) {
		return parser_parse_if_statement(parser, statement);
	}

	if (parser_check(parser, TOKEN_IDENTIFIER) &&
	    token_equals(&parser->current, "while")) {
		return parser_parse_while_statement(parser, statement);
	}

	if (parser_check(parser, TOKEN_IDENTIFIER) &&
	    token_equals(&parser->current, "return")) {
		return parser_parse_return_statement(parser, statement);
	}

	if (parser_check(parser, TOKEN_RIGHT_BRACE)) {
		parser_error_at(parser, &parser->current, "unexpected '}'");

		return false;
	}

	return parser_parse_expression_statement(parser, statement);
}

void declaration_destroy(Declaration *declaration) {
	statement_destroy(declaration->body);
	declaration->body = NULL;
}