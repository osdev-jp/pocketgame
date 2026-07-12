#ifndef POCKETGAME_EXPRESSION_H
#define POCKETGAME_EXPRESSION_H

#include "token.h"
#include <stdbool.h>
#include <stddef.h>

typedef enum {
	EXPRESSION_IDENTIFIER,
	EXPRESSION_INTEGER,
	EXPRESSION_BOOLEAN,
	EXPRESSION_STRING,
	EXPRESSION_CALL,
	EXPRESSION_UNARY,
	EXPRESSION_BINARY,
	EXPRESSION_TERNARY,
	EXPRESSION_ASSIGNMENT,
	EXPRESSION_POSTFIX,
} ExpressionType;

typedef struct Expression Expression;

struct Expression {
	ExpressionType type;
	size_t line;

	union {
		Token token;

		struct {
			Expression *callee;
			Expression **arguments;
			size_t argument_count;
			size_t argument_capacity;
		} call;

		struct {
			Token operator;
			Expression *operand;
		} unary;

		struct {
			Expression *left;
			Token operator;
			Expression *right;
		} binary;

		struct {
			Expression *condition;
			Expression *then_expression;
			Expression *else_expression;
		} ternary;

		struct {
			Expression *target;
			Token operator;
			Expression *value;
		} assignment;

		struct {
			Expression *operand;
			Token operator;
		} postfix;
	};
};

Expression *expression_parse(const char *input_path,
			     const Token *tokens,
			     size_t token_count,
			     bool *had_error);
void expression_destroy(Expression *expression);

#endif