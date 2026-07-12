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
static bool parser_is_type_name(const Token *token);
static bool parser_collect_parenthesized_tokens(Parser *parser,
						Token **tokens,
						size_t *token_count);

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

static bool parser_parse_directive(Parser *parser, Declaration *declaration) {
	Token *tokens;
	size_t token_count;

	declaration->type = DECLARATION_DIRECTIVE;
	declaration->name = parser->current;
	declaration->line = parser->current.line;
	declaration->body = NULL;
	declaration->tokens = NULL;
	declaration->token_count = 0;

	parser_advance(parser);

	tokens = NULL;
	token_count = 0;

	if (!parser_collect_parenthesized_tokens(parser, &tokens,
						 &token_count)) {
		return false;
	}

	if (!parser_consume(parser, TOKEN_SEMICOLON,
			    "expected ';' after directive")) {
		free(tokens);
		return false;
	}

	declaration->tokens = tokens;
	declaration->token_count = token_count;

	return true;
}

static bool parser_parse_function(Parser *parser, Declaration *declaration) {
	Statement *body;
	Token *parameters;
	Token return_type;
	Token name;
	size_t parameter_count;

	return_type = parser->current;
	parser_advance(parser);

	if (!parser_check(parser, TOKEN_IDENTIFIER)) {
		parser_error_at(parser, &parser->current,
				"expected function name");
		return false;
	}

	name = parser->current;
	parser_advance(parser);

	parameters = NULL;
	parameter_count = 0;

	if (!parser_collect_parenthesized_tokens(parser, &parameters,
						 &parameter_count)) {
		return false;
	}

	declaration->name = name;
	declaration->return_type = return_type;
	declaration->line = return_type.line;
	declaration->tokens = parameters;
	declaration->token_count = parameter_count;
	declaration->body = NULL;

	if (parser_match(parser, TOKEN_SEMICOLON)) {
		declaration->type = DECLARATION_FUNCTION_PROTOTYPE;
		return true;
	}

	if (parser_check(parser, TOKEN_LEFT_BRACE)) {
		body = NULL;

		if (!parser_parse_block_statement(parser, &body)) {
			free(parameters);
			declaration->tokens = NULL;
			declaration->token_count = 0;
			return false;
		}

		declaration->type = DECLARATION_FUNCTION_DEFINITION;
		declaration->body = body;
		return true;
	}

	parser_error_at(parser, &parser->current,
			"expected ';' or function body");

	free(parameters);
	declaration->tokens = NULL;
	declaration->token_count = 0;

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
	Statement *else_branch;
	Expression *condition;
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

	condition = expression_parse(parser->input_path, tokens, token_count,
				     &parser->had_error);

	free(tokens);

	if (condition == NULL) { return false; }

	then_branch = NULL;

	if (!parser_parse_statement(parser, &then_branch)) {
		expression_destroy(condition);
		return false;
	}

	else_branch = NULL;

	if (parser_check(parser, TOKEN_IDENTIFIER) &&
	    token_equals(&parser->current, "else")) {
		parser_advance(parser);

		if (!parser_parse_statement(parser, &else_branch)) {
			expression_destroy(condition);
			statement_destroy(then_branch);
			return false;
		}
	}

	if_statement = statement_create(STATEMENT_IF, line);

	if (if_statement == NULL) {
		parser_error_at(parser, &parser->current,
				"failed to allocate if statement");

		expression_destroy(condition);
		statement_destroy(then_branch);
		statement_destroy(else_branch);
		return false;
	}

	if_statement->if_statement.condition = condition;
	if_statement->if_statement.branch = then_branch;
	if_statement->if_statement.else_branch = else_branch;

	*statement = if_statement;
	return true;
}

static bool parser_parse_while_statement(Parser *parser,
					 Statement **statement) {
	Statement *while_statement;
	Statement *branch;
	Expression *condition;
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

	condition = expression_parse(parser->input_path, tokens, token_count,
				     &parser->had_error);

	free(tokens);

	if (condition == NULL) { return false; }

	branch = NULL;

	if (!parser_parse_statement(parser, &branch)) {
		expression_destroy(condition);
		return false;
	}

	while_statement = statement_create(STATEMENT_WHILE, line);

	if (while_statement == NULL) {
		parser_error_at(parser, &parser->current,
				"failed to allocate while statement");

		expression_destroy(condition);
		statement_destroy(branch);
		return false;
	}

	while_statement->while_statement.condition = condition;
	while_statement->while_statement.branch = branch;

	*statement = while_statement;
	return true;
}

static bool parser_find_for_separators(Parser *parser,
				       const Token *tokens,
				       size_t token_count,
				       size_t *first_separator,
				       size_t *second_separator) {
	size_t parenthesis_depth;
	size_t bracket_depth;
	size_t separator_count;
	size_t i;

	parenthesis_depth = 0;
	bracket_depth = 0;
	separator_count = 0;

	for (i = 0; i < token_count; i++) {
		switch (tokens[i].type) {
		case TOKEN_LEFT_PAREN: parenthesis_depth++; break;
		case TOKEN_RIGHT_PAREN:
			if (parenthesis_depth == 0) {
				parser_error_at(
					parser, &tokens[i],
					"unexpected ')' in for statement");
				return false;
			}

			parenthesis_depth--;
			break;
		case TOKEN_LEFT_BRACKET: bracket_depth++; break;
		case TOKEN_RIGHT_BRACKET:
			if (bracket_depth == 0) {
				parser_error_at(
					parser, &tokens[i],
					"unexpected ']' in for statement");
				return false;
			}

			bracket_depth--;
			break;
		case TOKEN_SEMICOLON:
			if (parenthesis_depth != 0 || bracket_depth != 0) {
				break;
			}

			if (separator_count == 0) {
				*first_separator = i;
			} else if (separator_count == 1) {
				*second_separator = i;
			}

			separator_count++;
			break;
		default: break;
		}
	}

	if (separator_count != 2) {
		parser_error_at(parser,
				token_count > 0 ? &tokens[0] : &parser->current,
				"for statement requires two ';' separators");
		return false;
	}

	return true;
}

static bool parser_parse_for_statement(Parser *parser, Statement **statement) {
	Statement *for_statement;
	Statement *branch;
	Expression *initializer;
	Expression *condition;
	Expression *update;
	Token *tokens;
	Token variable_type;
	Token variable_name;
	ForInitializerType initializer_type;
	size_t first_separator;
	size_t second_separator;
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

	if (!parser_find_for_separators(parser, tokens, token_count,
					&first_separator, &second_separator)) {
		free(tokens);
		return false;
	}

	initializer_type = FOR_INITIALIZER_NONE;
	memset(&variable_type, 0, sizeof(variable_type));
	memset(&variable_name, 0, sizeof(variable_name));
	initializer = NULL;
	condition = NULL;
	update = NULL;

	if (first_separator > 0) {
		if (first_separator >= 2 && parser_is_type_name(&tokens[0]) &&
		    tokens[1].type == TOKEN_IDENTIFIER) {
			initializer_type = FOR_INITIALIZER_VARIABLE;
			variable_type = tokens[0];
			variable_name = tokens[1];

			if (first_separator > 2) {
				if (tokens[2].type != TOKEN_ASSIGN) {
					parser_error_at(parser, &tokens[2],
							"expected '=' after "
							"for variable");

					free(tokens);
					return false;
				}

				if (first_separator == 3) {
					parser_error_at(parser, &tokens[2],
							"expected initializer "
							"after '='");

					free(tokens);
					return false;
				}

				initializer = expression_parse(
					parser->input_path, &tokens[3],
					first_separator - 3,
					&parser->had_error);

				if (initializer == NULL) {
					free(tokens);
					return false;
				}
			}
		} else {
			initializer_type = FOR_INITIALIZER_EXPRESSION;

			initializer = expression_parse(parser->input_path,
						       tokens, first_separator,
						       &parser->had_error);

			if (initializer == NULL) {
				free(tokens);
				return false;
			}
		}
	}

	if (second_separator > first_separator + 1) {
		condition = expression_parse(
			parser->input_path, &tokens[first_separator + 1],
			second_separator - first_separator - 1,
			&parser->had_error);

		if (condition == NULL) {
			expression_destroy(initializer);
			free(tokens);
			return false;
		}
	}

	if (token_count > second_separator + 1) {
		update = expression_parse(
			parser->input_path, &tokens[second_separator + 1],
			token_count - second_separator - 1, &parser->had_error);

		if (update == NULL) {
			expression_destroy(initializer);
			expression_destroy(condition);
			free(tokens);
			return false;
		}
	}

	free(tokens);

	branch = NULL;

	if (!parser_parse_statement(parser, &branch)) {
		expression_destroy(initializer);
		expression_destroy(condition);
		expression_destroy(update);
		return false;
	}

	for_statement = statement_create(STATEMENT_FOR, line);

	if (for_statement == NULL) {
		parser_error_at(parser, &parser->current,
				"failed to allocate for statement");

		expression_destroy(initializer);
		expression_destroy(condition);
		expression_destroy(update);
		statement_destroy(branch);
		return false;
	}

	for_statement->for_statement.initializer_type = initializer_type;
	for_statement->for_statement.variable_type = variable_type;
	for_statement->for_statement.variable_name = variable_name;
	for_statement->for_statement.initializer = initializer;
	for_statement->for_statement.condition = condition;
	for_statement->for_statement.update = update;
	for_statement->for_statement.branch = branch;

	*statement = for_statement;
	return true;
}

static bool parser_parse_return_statement(Parser *parser,
					  Statement **statement) {
	Statement *return_statement;
	Expression *value;
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

	value = NULL;

	if (token_count > 0) {
		value = expression_parse(parser->input_path, tokens,
					 token_count, &parser->had_error);

		if (value == NULL) {
			free(tokens);
			return false;
		}
	}

	free(tokens);

	return_statement = statement_create(STATEMENT_RETURN, line);

	if (return_statement == NULL) {
		parser_error_at(parser, &parser->current,
				"failed to allocate return statement");

		expression_destroy(value);
		return false;
	}

	return_statement->return_statement.value = value;

	*statement = return_statement;
	return true;
}

static bool parser_parse_jump_statement(Parser *parser,
					Statement **statement,
					const StatementType type) {
	Statement *jump_statement;
	size_t line;

	line = parser->current.line;
	parser_advance(parser);

	if (!parser_consume(parser, TOKEN_SEMICOLON,
			    "expected ';' after jump statement")) {
		return false;
	}

	jump_statement = statement_create(type, line);

	if (jump_statement == NULL) {
		parser_error_at(parser, &parser->current,
				"failed to allocate jump statement");
		return false;
	}

	*statement = jump_statement;
	return true;
}

static bool parser_parse_variable_statement(Parser *parser,
					    Statement **statement) {
	Statement *variable_statement;
	Expression *initializer;
	TokenList initializer_tokens;
	Token *tokens;
	Token type;
	Token name;
	size_t token_count;
	size_t line;

	type = parser->current;
	line = type.line;
	parser_advance(parser);

	if (!parser_check(parser, TOKEN_IDENTIFIER)) {
		parser_error_at(parser, &parser->current,
				"expected variable name");
		return false;
	}

	name = parser->current;
	parser_advance(parser);

	variable_statement = statement_create(STATEMENT_VARIABLE, line);

	if (variable_statement == NULL) {
		parser_error_at(parser, &parser->current,
				"failed to allocate variable statement");
		return false;
	}

	variable_statement->variable.type = type;
	variable_statement->variable.name = name;
	variable_statement->variable.initializer = NULL;

	if (parser_match(parser, TOKEN_SEMICOLON)) {
		*statement = variable_statement;
		return true;
	}

	if (!parser_consume(parser, TOKEN_ASSIGN,
			    "expected '=' or ';' after variable name")) {
		statement_destroy(variable_statement);
		return false;
	}

	token_list_init(&initializer_tokens);

	while (!parser_check(parser, TOKEN_SEMICOLON) &&
	       !parser_check(parser, TOKEN_EOF)) {
		if (parser_check(parser, TOKEN_LEFT_BRACE) ||
		    parser_check(parser, TOKEN_RIGHT_BRACE)) {
			parser_error_at(
				parser, &parser->current,
				"expected ';' after variable declaration");

			token_list_destroy(&initializer_tokens);
			statement_destroy(variable_statement);
			return false;
		}

		if (!token_list_append(parser, &initializer_tokens,
				       parser->current)) {
			token_list_destroy(&initializer_tokens);
			statement_destroy(variable_statement);
			return false;
		}

		parser_advance(parser);
	}

	if (!parser_consume(parser, TOKEN_SEMICOLON,
			    "expected ';' after variable declaration")) {
		token_list_destroy(&initializer_tokens);
		statement_destroy(variable_statement);
		return false;
	}

	tokens = token_list_take(&initializer_tokens, &token_count);

	initializer = expression_parse(parser->input_path, tokens, token_count,
				       &parser->had_error);

	free(tokens);

	if (initializer == NULL) {
		statement_destroy(variable_statement);
		return false;
	}

	variable_statement->variable.initializer = initializer;

	*statement = variable_statement;
	return true;
}

static bool parser_parse_expression_statement(Parser *parser,
					      Statement **statement) {
	Statement *expression_statement;
	Expression *value;
	Token *tokens;
	size_t token_count;
	size_t line;

	line = parser->current.line;
	tokens = NULL;
	token_count = 0;

	if (!parser_collect_until_semicolon(parser, &tokens, &token_count)) {
		return false;
	}

	if (token_count == 0) {
		parser_error_at(parser, &parser->previous,
				"expected expression");

		free(tokens);
		return false;
	}

	value = expression_parse(parser->input_path, tokens, token_count,
				 &parser->had_error);

	free(tokens);

	if (value == NULL) { return false; }

	expression_statement = statement_create(STATEMENT_EXPRESSION, line);

	if (expression_statement == NULL) {
		parser_error_at(parser, &parser->current,
				"failed to allocate expression statement");

		expression_destroy(value);
		return false;
	}

	expression_statement->expression.value = value;

	*statement = expression_statement;
	return true;
}

static bool parser_is_type_name(const Token *token) {
	if (token->type != TOKEN_IDENTIFIER) return false;

	// TODO: 他の型も忘れてたら実装
	return token_equals(token, "int") || token_equals(token, "bool") ||
	       token_equals(token, "void") || token_equals(token, "char") ||
	       token_equals(token, "float") || token_equals(token, "double");
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
	    token_equals(&parser->current, "for")) {
		return parser_parse_for_statement(parser, statement);
	}

	if (parser_check(parser, TOKEN_IDENTIFIER) &&
	    token_equals(&parser->current, "continue")) {
		return parser_parse_jump_statement(parser, statement,
						   STATEMENT_CONTINUE);
	}

	if (parser_check(parser, TOKEN_IDENTIFIER) &&
	    token_equals(&parser->current, "break")) {
		return parser_parse_jump_statement(parser, statement,
						   STATEMENT_BREAK);
	}

	if (parser_check(parser, TOKEN_IDENTIFIER) &&
	    token_equals(&parser->current, "return")) {
		return parser_parse_return_statement(parser, statement);
	}

	if (parser_is_type_name(&parser->current)) {
		return parser_parse_variable_statement(parser, statement);
	}

	if (parser_check(parser, TOKEN_RIGHT_BRACE)) {
		parser_error_at(parser, &parser->current, "unexpected '}'");

		return false;
	}

	return parser_parse_expression_statement(parser, statement);
}

void declaration_destroy(Declaration *declaration) {
	free(declaration->tokens);
	statement_destroy(declaration->body);
	declaration->tokens = NULL;
	declaration->token_count = 0;
	declaration->body = NULL;
}
