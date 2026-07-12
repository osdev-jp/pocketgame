#include "debug.h"
#include <stdio.h>

static void ast_dump_indent(size_t depth) {
	size_t i;

	for (i = 0; i < depth; i++) {
		fputs("  ", stdout);
	}
}

static void ast_dump_expression(const Expression *expression, size_t depth) {
	size_t i;

	if (expression == NULL) {
		ast_dump_indent(depth);
		puts("<null expression>");
		return;
	}

	switch (expression->type) {
	case EXPRESSION_IDENTIFIER:
		ast_dump_indent(depth);
		printf("IDENTIFIER %.*s\n", (int)expression->token.length,
		       expression->token.start);
		break;

	case EXPRESSION_INTEGER:
		ast_dump_indent(depth);
		printf("INTEGER %.*s\n", (int)expression->token.length,
		       expression->token.start);
		break;

	case EXPRESSION_BOOLEAN:
		ast_dump_indent(depth);
		printf("BOOLEAN %.*s\n", (int)expression->token.length,
		       expression->token.start);
		break;

	case EXPRESSION_STRING:
		ast_dump_indent(depth);
		printf("STRING \"%.*s\"\n", (int)expression->token.length,
		       expression->token.start);
		break;

	case EXPRESSION_CALL:
		ast_dump_indent(depth);
		puts("CALL");

		ast_dump_indent(depth + 1);
		puts("CALLEE:");
		ast_dump_expression(expression->call.callee, depth + 2);

		for (i = 0; i < expression->call.argument_count; i++) {
			ast_dump_indent(depth + 1);
			printf("ARGUMENT %zu:\n", i);

			ast_dump_expression(expression->call.arguments[i],
					    depth + 2);
		}
		break;

	case EXPRESSION_UNARY:
		ast_dump_indent(depth);
		printf("UNARY %.*s\n", (int)expression->unary.operator.length,
		       expression->unary.operator.start);

		ast_dump_expression(expression->unary.operand, depth + 1);
		break;

	case EXPRESSION_BINARY:
		ast_dump_indent(depth);
		printf("BINARY %.*s\n", (int)expression->binary.operator.length,
		       expression->binary.operator.start);

		ast_dump_expression(expression->binary.left, depth + 1);
		ast_dump_expression(expression->binary.right, depth + 1);
		break;

	case EXPRESSION_TERNARY:
		ast_dump_indent(depth);
		puts("TERNARY");

		ast_dump_indent(depth + 1);
		puts("CONDITION:");
		ast_dump_expression(expression->ternary.condition, depth + 2);

		ast_dump_indent(depth + 1);
		puts("THEN:");
		ast_dump_expression(expression->ternary.then_expression,
				    depth + 2);

		ast_dump_indent(depth + 1);
		puts("ELSE:");
		ast_dump_expression(expression->ternary.else_expression,
				    depth + 2);
		break;

	case EXPRESSION_ASSIGNMENT:
		ast_dump_indent(depth);
		printf("ASSIGNMENT %.*s\n",
		       (int)expression->assignment.operator.length,
		       expression->assignment.operator.start);

		ast_dump_expression(expression->assignment.target, depth + 1);
		ast_dump_expression(expression->assignment.value, depth + 1);
		break;

	case EXPRESSION_POSTFIX:
		ast_dump_indent(depth);
		printf("POSTFIX %.*s\n",
		       (int)expression->postfix.operator.length,
		       expression->postfix.operator.start);

		ast_dump_expression(expression->postfix.operand, depth + 1);
		break;
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
		puts("EXPRESSION");

		ast_dump_expression(statement->expression.value, depth + 1);
		break;

	case STATEMENT_VARIABLE:
		ast_dump_indent(depth);

		printf("VARIABLE %.*s %.*s\n",
		       (int)statement->variable.type.length,
		       statement->variable.type.start,
		       (int)statement->variable.name.length,
		       statement->variable.name.start);

		if (statement->variable.initializer != NULL) {
			ast_dump_indent(depth + 1);
			puts("INITIALIZER:");

			ast_dump_expression(statement->variable.initializer,
					    depth + 2);
		}
		break;

	case STATEMENT_RETURN:
		ast_dump_indent(depth);
		puts("RETURN");

		if (statement->return_statement.value != NULL) {
			ast_dump_expression(statement->return_statement.value,
					    depth + 1);
		}
		break;

	case STATEMENT_IF:
		ast_dump_indent(depth);
		puts("IF");

		ast_dump_expression(statement->if_statement.condition,
				    depth + 1);

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
		puts("WHILE");

		ast_dump_expression(statement->while_statement.condition,
				    depth + 1);

		ast_dump_statement(statement->while_statement.branch,
				   depth + 1);
		break;

	case STATEMENT_FOR:
		ast_dump_indent(depth);
		puts("FOR");

		if (statement->for_statement.initializer_type ==
		    FOR_INITIALIZER_VARIABLE) {
			ast_dump_indent(depth + 1);

			printf("VARIABLE %.*s %.*s\n",
			       (int)statement->for_statement.variable_type
				       .length,
			       statement->for_statement.variable_type.start,
			       (int)statement->for_statement.variable_name
				       .length,
			       statement->for_statement.variable_name.start);

			if (statement->for_statement.initializer != NULL) {
				ast_dump_indent(depth + 2);
				puts("INITIALIZER:");

				ast_dump_expression(
					statement->for_statement.initializer,
					depth + 3);
			}
		} else if (statement->for_statement.initializer_type ==
			   FOR_INITIALIZER_EXPRESSION) {
			ast_dump_indent(depth + 1);
			puts("INITIALIZER:");

			ast_dump_expression(
				statement->for_statement.initializer,
				depth + 2);
		}

		if (statement->for_statement.condition != NULL) {
			ast_dump_indent(depth + 1);
			puts("CONDITION:");

			ast_dump_expression(statement->for_statement.condition,
					    depth + 2);
		}

		if (statement->for_statement.update != NULL) {
			ast_dump_indent(depth + 1);
			puts("UPDATE:");

			ast_dump_expression(statement->for_statement.update,
					    depth + 2);
		}

		ast_dump_indent(depth + 1);
		puts("BODY:");

		ast_dump_statement(statement->for_statement.branch, depth + 2);
		break;

	case STATEMENT_BLOCK:
		ast_dump_indent(depth);
		puts("BLOCK");

		for (i = 0; i < statement->block.count; i++) {
			ast_dump_statement(statement->block.items[i],
					   depth + 1);
		}
		break;

	case STATEMENT_CONTINUE:
		ast_dump_indent(depth);
		puts("CONTINUE");
		break;

	case STATEMENT_BREAK:
		ast_dump_indent(depth);
		puts("BREAK");
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