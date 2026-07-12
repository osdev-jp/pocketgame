#include "ast.h"

#include <stdlib.h>

void statement_list_init(StatementList *list) {
	list->items = NULL;
	list->count = 0;
	list->capacity = 0;
}

void statement_list_destroy(StatementList *list) {
	size_t i;

	for (i = 0; i < list->count; i++) {
		statement_destroy(list->items[i]);
	}

	free(list->items);

	statement_list_init(list);
}

int statement_list_append(StatementList *list, Statement *item) {
	Statement **items;
	size_t capacity;

	if (list->count == list->capacity) {
		capacity = list->capacity == 0 ? 8 : list->capacity * 2;

		items = realloc(list->items, capacity * sizeof(*list->items));

		if (items == NULL) return -1;

		list->items = items;
		list->capacity = capacity;
	}

	list->items[list->count] = item;
	list->count++;

	return 0;
}

Statement *statement_create(const StatementType type, const size_t line) {
	Statement *statement;
	statement = calloc(1, sizeof(*statement));

	if (statement == NULL) return NULL;

	statement->type = type;
	statement->line = line;

	if (type == STATEMENT_BLOCK) { statement_list_init(&statement->block); }

	return statement;
}

void statement_destroy(Statement *statement) {
	if (statement == NULL) return;

	switch (statement->type) {
	case STATEMENT_EXPRESSION: free(statement->expression.tokens); break;
	case STATEMENT_VARIABLE: free(statement->variable.tokens); break;
	case STATEMENT_RETURN: free(statement->return_statement.tokens); break;
	case STATEMENT_BREAK:
	case STATEMENT_CONTINUE: break;
	case STATEMENT_IF:
		free(statement->if_statement.tokens);
		statement_destroy(statement->if_statement.branch);
		statement_destroy(statement->if_statement.else_branch);
		break;
	case STATEMENT_WHILE:
		free(statement->while_statement.tokens);
		statement_destroy(statement->while_statement.branch);
		break;
	case STATEMENT_FOR:
		free(statement->for_statement.tokens);
		statement_destroy(statement->for_statement.branch);
		break;
	case STATEMENT_BLOCK: statement_list_destroy(&statement->block); break;
	}

	free(statement);
}
