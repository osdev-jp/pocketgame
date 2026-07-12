#ifndef POCKETGAME_AST_H
#define POCKETGAME_AST_H
#include "token.h"

#include <stddef.h>

typedef enum {
	STATEMENT_EXPRESSION,
	STATEMENT_RETURN,
	STATEMENT_IF,
	STATEMENT_WHILE,
	STATEMENT_FOR,
	STATEMENT_BLOCK,
} StatementType;

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
			Token *tokens;
			size_t token_count;
		} expression;

		struct {
			Token *tokens;
			size_t token_count;
		} return_statement;

		struct {
			Token *tokens;
			size_t token_count;
			Statement *branch;
			Statement *else_branch;
		} if_statement;

		struct {
			Token *tokens;
			size_t token_count;
			Statement *branch;
		} for_statement;

		struct {
			Token *tokens;
			size_t token_count;
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
