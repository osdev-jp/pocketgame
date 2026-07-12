#include "expression.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	const char *input_path;
	const Token *tokens;
	size_t token_count;
	size_t current;
	bool had_error;
} ExpressionParser;

typedef Expression *(*ParseFunction)(ExpressionParser *parser);

static Expression *parse_assignment(ExpressionParser *parser);

static bool token_equals(const Token *token, const char *text) {
	size_t length;

	length = strlen(text);

	if (token->length != length) { return false; }

	return memcmp(token->start, text, length) == 0;
}

static bool expression_parser_is_at_end(const ExpressionParser *parser) {
	return parser->current >= parser->token_count;
}

static const Token *expression_parser_peek(const ExpressionParser *parser) {
	if (expression_parser_is_at_end(parser)) { return NULL; }

	return &parser->tokens[parser->current];
}

static const Token *expression_parser_previous(const ExpressionParser *parser) {
	if (parser->current == 0) { return NULL; }

	return &parser->tokens[parser->current - 1];
}

static void expression_parser_error_at(ExpressionParser *parser,
				       const Token *token,
				       const char *message) {
	size_t line;
	size_t column;

	if (parser->had_error) { return; }

	line = 1;
	column = 1;

	if (token != NULL) {
		line = token->line;
		column = token->column;
	} else if (parser->token_count > 0) {
		line = parser->tokens[parser->token_count - 1].line;
		column = parser->tokens[parser->token_count - 1].column;
	}

	fprintf(stderr, "%s:%zu:%zu: %s", parser->input_path, line, column,
		message);

	if (token != NULL) {
		fprintf(stderr, " near '%.*s'", (int)token->length,
			token->start);
	}

	fputc('\n', stderr);
	parser->had_error = true;
}

static const Token *expression_parser_advance(ExpressionParser *parser) {
	if (!expression_parser_is_at_end(parser)) { parser->current++; }

	return expression_parser_previous(parser);
}

static bool expression_parser_check(const ExpressionParser *parser,
				    TokenType type) {
	const Token *token;

	token = expression_parser_peek(parser);

	if (token == NULL) { return false; }

	return token->type == type;
}

static bool expression_parser_match(ExpressionParser *parser, TokenType type) {
	if (!expression_parser_check(parser, type)) { return false; }

	expression_parser_advance(parser);
	return true;
}

static const Token *expression_parser_consume(ExpressionParser *parser,
					      TokenType type,
					      const char *message) {
	const Token *token;

	if (!expression_parser_check(parser, type)) {
		expression_parser_error_at(
			parser, expression_parser_peek(parser), message);
		return NULL;
	}

	token = expression_parser_peek(parser);
	expression_parser_advance(parser);
	return token;
}

static Expression *
expression_create(ExpressionParser *parser, ExpressionType type, size_t line) {
	Expression *expression;

	expression = calloc(1, sizeof(*expression));

	if (expression == NULL) {
		expression_parser_error_at(parser,
					   expression_parser_peek(parser),
					   "failed to allocate expression");
		return NULL;
	}

	expression->type = type;
	expression->line = line;
	return expression;
}

static bool expression_call_append(ExpressionParser *parser,
				   Expression *call,
				   Expression *argument) {
	Expression **arguments;
	size_t capacity;

	if (call->call.argument_count == call->call.argument_capacity) {
		capacity = call->call.argument_capacity == 0
				   ? 4
				   : call->call.argument_capacity * 2;

		arguments = realloc(call->call.arguments,
				    capacity * sizeof(*arguments));

		if (arguments == NULL) {
			expression_parser_error_at(
				parser, expression_parser_peek(parser),
				"failed to allocate call arguments");
			return false;
		}

		call->call.arguments = arguments;
		call->call.argument_capacity = capacity;
	}

	call->call.arguments[call->call.argument_count] = argument;
	call->call.argument_count++;

	return true;
}

static Expression *parse_primary(ExpressionParser *parser) {
	Expression *expression;
	const Token *token;

	if (expression_parser_match(parser, TOKEN_INTEGER)) {
		token = expression_parser_previous(parser);

		expression = expression_create(parser, EXPRESSION_INTEGER,
					       token->line);

		if (expression != NULL) { expression->token = *token; }

		return expression;
	}

	if (expression_parser_match(parser, TOKEN_STRING)) {
		token = expression_parser_previous(parser);

		expression = expression_create(parser, EXPRESSION_STRING,
					       token->line);

		if (expression != NULL) { expression->token = *token; }

		return expression;
	}

	if (expression_parser_match(parser, TOKEN_IDENTIFIER)) {
		token = expression_parser_previous(parser);

		if (token_equals(token, "true") ||
		    token_equals(token, "false")) {
			expression = expression_create(
				parser, EXPRESSION_BOOLEAN, token->line);
		} else {
			expression = expression_create(
				parser, EXPRESSION_IDENTIFIER, token->line);
		}

		if (expression != NULL) { expression->token = *token; }

		return expression;
	}

	if (expression_parser_match(parser, TOKEN_LEFT_PAREN)) {
		expression = parse_assignment(parser);

		if (expression == NULL) { return NULL; }

		if (expression_parser_consume(parser, TOKEN_RIGHT_PAREN,
					      "expected ')'") == NULL) {
			expression_destroy(expression);
			return NULL;
		}

		return expression;
	}

	expression_parser_error_at(parser, expression_parser_peek(parser),
				   "expected expression");

	return NULL;
}

static Expression *parse_postfix(ExpressionParser *parser) {
	Expression *expression;

	expression = parse_primary(parser);

	if (expression == NULL) { return NULL; }

	for (;;) {
		if (expression_parser_match(parser, TOKEN_LEFT_PAREN)) {
			Expression *call;

			call = expression_create(parser, EXPRESSION_CALL,
						 expression->line);

			if (call == NULL) {
				expression_destroy(expression);
				return NULL;
			}

			call->call.callee = expression;

			if (!expression_parser_check(parser,
						     TOKEN_RIGHT_PAREN)) {
				for (;;) {
					Expression *argument;

					argument = parse_assignment(parser);

					if (argument == NULL) {
						expression_destroy(call);
						return NULL;
					}

					if (!expression_call_append(
						    parser, call, argument)) {
						expression_destroy(argument);
						expression_destroy(call);
						return NULL;
					}

					if (!expression_parser_match(
						    parser, TOKEN_COMMA)) {
						break;
					}
				}
			}

			if (expression_parser_consume(
				    parser, TOKEN_RIGHT_PAREN,
				    "expected ')' after arguments") == NULL) {
				expression_destroy(call);
				return NULL;
			}

			expression = call;
			continue;
		}

		if (expression_parser_match(parser, TOKEN_INCREMENT) ||
		    expression_parser_match(parser, TOKEN_DECREMENT)) {
			Expression *postfix;
			const Token *operator;

			operator= expression_parser_previous(parser);

			postfix = expression_create(
				parser, EXPRESSION_POSTFIX, operator->line);

			if (postfix == NULL) {
				expression_destroy(expression);
				return NULL;
			}

			postfix->postfix.operand = expression;
			postfix->postfix.operator= * operator;
			expression = postfix;
			continue;
		}

		break;
	}

	return expression;
}

static Expression *parse_unary(ExpressionParser *parser) {
	Expression *expression;
	Expression *operand;
	const Token *operator;

	if (expression_parser_match(parser, TOKEN_NOT) ||
	    expression_parser_match(parser, TOKEN_MINUS) ||
	    expression_parser_match(parser, TOKEN_STAR) ||
	    expression_parser_match(parser, TOKEN_AND)) {
		operator= expression_parser_previous(parser);
		operand = parse_unary(parser);

		if (operand == NULL) { return NULL; }

		expression = expression_create(
			parser, EXPRESSION_UNARY, operator->line);

		if (expression == NULL) {
			expression_destroy(operand);
			return NULL;
		}

		expression->unary.operator= * operator;
		expression->unary.operand = operand;
		return expression;
	}

	return parse_postfix(parser);
}

static bool expression_parser_match_any(ExpressionParser *parser,
					const TokenType *types,
					size_t type_count,
					Token *operator) {
	size_t i;

	for (i = 0; i < type_count; i++) {
		if (expression_parser_match(parser, types[i])) {
			*operator= * expression_parser_previous(parser);
			return true;
		}
	}

	return false;
}

static Expression *parse_binary(ExpressionParser *parser,
				ParseFunction lower,
				const TokenType *operators,
				size_t operator_count) {
	Expression *left;
	Token operator;

	left = lower(parser);

	if (left == NULL) { return NULL; }

	while (expression_parser_match_any(parser, operators, operator_count,
					   &operator)) {
		Expression *binary;
		Expression *right;

		right = lower(parser);

		if (right == NULL) {
			expression_destroy(left);
			return NULL;
		}

		binary = expression_create(parser,
					   EXPRESSION_BINARY, operator.line);

		if (binary == NULL) {
			expression_destroy(left);
			expression_destroy(right);
			return NULL;
		}

		binary->binary.left = left;
		binary->binary.operator= operator;
		binary->binary.right = right;
		left = binary;
	}

	return left;
}

static Expression *parse_factor(ExpressionParser *parser) {
	static const TokenType operators[] = {
		TOKEN_STAR,
		TOKEN_SLASH,
		TOKEN_PERCENT,
	};

	return parse_binary(parser, parse_unary, operators,
			    sizeof(operators) / sizeof(operators[0]));
}

static Expression *parse_term(ExpressionParser *parser) {
	static const TokenType operators[] = {
		TOKEN_PLUS,
		TOKEN_MINUS,
	};

	return parse_binary(parser, parse_factor, operators,
			    sizeof(operators) / sizeof(operators[0]));
}

static Expression *parse_comparison(ExpressionParser *parser) {
	static const TokenType operators[] = {
		TOKEN_LESS,
		TOKEN_LESS_EQUAL,
		TOKEN_GREATER,
		TOKEN_GREATER_EQUAL,
	};

	return parse_binary(parser, parse_term, operators,
			    sizeof(operators) / sizeof(operators[0]));
}

static Expression *parse_equality(ExpressionParser *parser) {
	static const TokenType operators[] = {
		TOKEN_EQUAL,
		TOKEN_NOT_EQUAL,
	};

	return parse_binary(parser, parse_comparison, operators,
			    sizeof(operators) / sizeof(operators[0]));
}

static Expression *parse_logical_and(ExpressionParser *parser) {
	static const TokenType operators[] = {
		TOKEN_LOGICAL_AND,
	};

	return parse_binary(parser, parse_equality, operators,
			    sizeof(operators) / sizeof(operators[0]));
}

static Expression *parse_logical_or(ExpressionParser *parser) {
	static const TokenType operators[] = {
		TOKEN_LOGICAL_OR,
	};

	return parse_binary(parser, parse_logical_and, operators,
			    sizeof(operators) / sizeof(operators[0]));
}

static Expression *parse_ternary(ExpressionParser *parser) {
	Expression *condition;

	condition = parse_logical_or(parser);

	if (condition == NULL) { return NULL; }

	if (expression_parser_match(parser, TOKEN_QUESTION)) {
		Expression *expression;
		Expression *then_expression;
		Expression *else_expression;
		const Token *question;

		question = expression_parser_previous(parser);
		then_expression = parse_assignment(parser);

		if (then_expression == NULL) {
			expression_destroy(condition);
			return NULL;
		}

		if (expression_parser_consume(
			    parser, TOKEN_COLON,
			    "expected ':' in ternary expression") == NULL) {
			expression_destroy(condition);
			expression_destroy(then_expression);
			return NULL;
		}

		else_expression = parse_assignment(parser);

		if (else_expression == NULL) {
			expression_destroy(condition);
			expression_destroy(then_expression);
			return NULL;
		}

		expression = expression_create(parser, EXPRESSION_TERNARY,
					       question->line);

		if (expression == NULL) {
			expression_destroy(condition);
			expression_destroy(then_expression);
			expression_destroy(else_expression);
			return NULL;
		}

		expression->ternary.condition = condition;
		expression->ternary.then_expression = then_expression;
		expression->ternary.else_expression = else_expression;

		return expression;
	}

	return condition;
}

static bool is_assignment_operator(TokenType type) {
	return type == TOKEN_ASSIGN || type == TOKEN_PLUS_ASSIGN ||
	       type == TOKEN_MINUS_ASSIGN || type == TOKEN_STAR_ASSIGN ||
	       type == TOKEN_SLASH_ASSIGN;
}

static Expression *parse_assignment(ExpressionParser *parser) {
	Expression *target;
	const Token *token;

	target = parse_ternary(parser);

	if (target == NULL) { return NULL; }

	token = expression_parser_peek(parser);

	if (token != NULL && is_assignment_operator(token->type)) {
		Expression *assignment;
		Expression *value;
		Token operator;

		operator= * token;
		expression_parser_advance(parser);

		value = parse_assignment(parser);

		if (value == NULL) {
			expression_destroy(target);
			return NULL;
		}

		assignment = expression_create(
			parser, EXPRESSION_ASSIGNMENT, operator.line);

		if (assignment == NULL) {
			expression_destroy(target);
			expression_destroy(value);
			return NULL;
		}

		assignment->assignment.target = target;
		assignment->assignment.operator= operator;
		assignment->assignment.value = value;

		return assignment;
	}

	return target;
}

Expression *expression_parse(const char *input_path,
			     const Token *tokens,
			     size_t token_count,
			     bool *had_error) {
	ExpressionParser parser;
	Expression *expression;

	parser.input_path = input_path;
	parser.tokens = tokens;
	parser.token_count = token_count;
	parser.current = 0;
	parser.had_error = false;

	expression = parse_assignment(&parser);

	if (expression != NULL && !expression_parser_is_at_end(&parser)) {
		expression_parser_error_at(&parser,
					   expression_parser_peek(&parser),
					   "unexpected token after expression");

		expression_destroy(expression);
		expression = NULL;
	}

	if (parser.had_error && expression != NULL) {
		expression_destroy(expression);
		expression = NULL;
	}

	if (parser.had_error && had_error != NULL) { *had_error = true; }

	return expression;
}

void expression_destroy(Expression *expression) {
	size_t i;

	if (expression == NULL) { return; }

	switch (expression->type) {
	case EXPRESSION_IDENTIFIER:
	case EXPRESSION_INTEGER:
	case EXPRESSION_BOOLEAN:
	case EXPRESSION_STRING: break;

	case EXPRESSION_CALL:
		expression_destroy(expression->call.callee);

		for (i = 0; i < expression->call.argument_count; i++) {
			expression_destroy(expression->call.arguments[i]);
		}

		free(expression->call.arguments);
		break;

	case EXPRESSION_UNARY:
		expression_destroy(expression->unary.operand);
		break;

	case EXPRESSION_BINARY:
		expression_destroy(expression->binary.left);
		expression_destroy(expression->binary.right);
		break;

	case EXPRESSION_TERNARY:
		expression_destroy(expression->ternary.condition);
		expression_destroy(expression->ternary.then_expression);
		expression_destroy(expression->ternary.else_expression);
		break;

	case EXPRESSION_ASSIGNMENT:
		expression_destroy(expression->assignment.target);
		expression_destroy(expression->assignment.value);
		break;

	case EXPRESSION_POSTFIX:
		expression_destroy(expression->postfix.operand);
		break;
	}

	free(expression);
}