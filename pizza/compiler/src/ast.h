#ifndef POCKETGAME_AST_H
#define POCKETGAME_AST_H

#include "expression.h"
#include "token.h"
#include <stddef.h>

typedef enum {
	STATEMENT_EXPRESSION,
	STATEMENT_VARIABLE,
	STATEMENT_RETURN,
	STATEMENT_CONTINUE,
	STATEMENT_BREAK,
	STATEMENT_IF,
	STATEMENT_WHILE,
	STATEMENT_FOR,
	STATEMENT_BLOCK,
} StatementType;

typedef enum {
	FOR_INITIALIZER_NONE,
	FOR_INITIALIZER_EXPRESSION,
	FOR_INITIALIZER_VARIABLE,
} ForInitializerType;

typedef struct Statement Statement;

typedef struct {
	Statement **items;
	size_t count;
	size_t capacity;
} StatementList;

struct Statement {
	StatementType type;
	size_t line;

	union {
		struct {
			Expression *value;
		} expression;

		struct {
			Token type;
			Token name;
			Expression *initializer;
		} variable;

		struct {
			Expression *value;
		} return_statement;

		struct {
			Expression *condition;
			Statement *branch;
			Statement *else_branch;
		} if_statement;

		struct {
			ForInitializerType initializer_type;
			Token variable_type;
			Token variable_name;
			Expression *initializer;
			Expression *condition;
			Expression *update;
			Statement *branch;
		} for_statement;

		struct {
			Expression *condition;
			Statement *branch;
		} while_statement;

		StatementList block;
	};
};

void statement_list_init(StatementList *list);
void statement_list_destroy(StatementList *list);
int statement_list_append(StatementList *list, Statement *item);
Statement *statement_create(StatementType type, size_t line);
void statement_destroy(Statement *statement);

#endif
