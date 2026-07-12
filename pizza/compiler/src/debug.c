#include "debug.h"
#include <stdio.h>

static void ast_dump_indent(size_t depth) {
	size_t i;

	for (i = 0; i < depth; i++) {
		fputs("  ", stdout);
	}
}

static void ast_dump_tokens(const Token *tokens, size_t token_count) {
	size_t i;

	for (i = 0; i < token_count; i++) {
		if (i > 0) { putchar(' '); }

		if (tokens[i].type == TOKEN_STRING) {
			printf("\"%.*s\"", (int)tokens[i].length,
			       tokens[i].start);
			continue;
		}

		printf("%.*s", (int)tokens[i].length, tokens[i].start);
	}
}

static void ast_dump_statement(const Statement *statement, size_t depth) {
	size_t i;

	if (statement == NULL) {
		ast_dump_indent(depth);
		puts("<null>");
		return;
	}

	switch (statement->type) {
	case STATEMENT_EXPRESSION:
		ast_dump_indent(depth);
		fputs("EXPRESSION: ", stdout);
		ast_dump_tokens(statement->expression.tokens,
				statement->expression.token_count);
		putchar('\n');
		break;

	case STATEMENT_VARIABLE:
		ast_dump_indent(depth);

		printf("VARIABLE: %.*s %.*s",
		       (int)statement->variable.type.length,
		       statement->variable.type.start,
		       (int)statement->variable.name.length,
		       statement->variable.name.start);

		if (statement->variable.token_count > 0) {
			fputs(" = ", stdout);

			ast_dump_tokens(statement->variable.tokens,
					statement->variable.token_count);
		}

		putchar('\n');
		break;

	case STATEMENT_RETURN:
		ast_dump_indent(depth);

		if (statement->return_statement.token_count == 0) {
			puts("RETURN");
			break;
		}

		fputs("RETURN: ", stdout);
		ast_dump_tokens(statement->return_statement.tokens,
				statement->return_statement.token_count);
		putchar('\n');
		break;

	case STATEMENT_CONTINUE:
		ast_dump_indent(depth);
		puts("CONTINUE");
		break;

	case STATEMENT_BREAK:
		ast_dump_indent(depth);
		puts("BREAK");
		break;

	case STATEMENT_IF:
		ast_dump_indent(depth);
		fputs("IF: ", stdout);
		ast_dump_tokens(statement->if_statement.tokens,
				statement->if_statement.token_count);
		putchar('\n');

		ast_dump_indent(depth);
		puts("THEN:");
		ast_dump_statement(statement->if_statement.branch, depth + 1);

		if (statement->if_statement.else_branch != NULL) {
			ast_dump_indent(depth);
			puts("ELSE:");
			ast_dump_statement(statement->if_statement.else_branch,
					   depth + 1);
		}
		break;

	case STATEMENT_WHILE:
		ast_dump_indent(depth);
		fputs("WHILE: ", stdout);
		ast_dump_tokens(statement->while_statement.tokens,
				statement->while_statement.token_count);
		putchar('\n');

		ast_dump_statement(statement->while_statement.branch,
				   depth + 1);
		break;

	case STATEMENT_FOR:
		ast_dump_indent(depth);
		fputs("FOR: ", stdout);
		ast_dump_tokens(statement->for_statement.tokens,
				statement->for_statement.token_count);
		putchar('\n');

		ast_dump_statement(statement->for_statement.branch, depth + 1);
		break;

	case STATEMENT_BLOCK:
		ast_dump_indent(depth);
		puts("BLOCK");

		for (i = 0; i < statement->block.count; i++) {
			ast_dump_statement(statement->block.items[i],
					   depth + 1);
		}
		break;
	}
}

void ast_dump_declaration(const Declaration *declaration) {
	switch (declaration->type) {
	case DECLARATION_DIRECTIVE:
		printf("DIRECTIVE %.*s\n", (int)declaration->name.length,
		       declaration->name.start);
		break;

	case DECLARATION_FUNCTION_PROTOTYPE:
		printf("PROTOTYPE %.*s -> %.*s\n",
		       (int)declaration->name.length, declaration->name.start,
		       (int)declaration->return_type.length,
		       declaration->return_type.start);
		break;

	case DECLARATION_FUNCTION_DEFINITION:
		printf("FUNCTION %.*s -> %.*s\n", (int)declaration->name.length,
		       declaration->name.start,
		       (int)declaration->return_type.length,
		       declaration->return_type.start);

		ast_dump_statement(declaration->body, 1);
		break;
	}
}