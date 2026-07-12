#include "codegen.h"
#include <stdio.h>

static void codegen_emit_expression(CodeGenerator *generator,
				    const Expression *expression);

static void codegen_emit_statement(CodeGenerator *generator,
				   const Statement *statement);

static void codegen_error(CodeGenerator *generator,
			  const size_t line,
			  const char *message) {
	fprintf(stderr, "codegen:%zu: %s\n", line, message);
	generator->had_error = true;
}

static void codegen_emit_indent(const CodeGenerator *generator) {
	size_t i;

	for (i = 0; i < generator->indent; i++) {
		fputc('\t', generator->output);
	}
}

static void codegen_emit_token(const CodeGenerator *generator,
			       const Token *token) {
	if (token->type == TOKEN_STRING) {
		fputc('"', generator->output);
		fwrite(token->start, 1, token->length, generator->output);
		fputc('"', generator->output);
		return;
	}

	fwrite(token->start, 1, token->length, generator->output);
}

static void codegen_emit_tokens(const CodeGenerator *generator,
				const Token *tokens,
				const size_t token_count) {
	size_t i;

	for (i = 0; i < token_count; i++) {
		codegen_emit_token(generator, &tokens[i]);

		if (i + 1 < token_count) { fputc(' ', generator->output); }
	}
}

static size_t codegen_allocate_label(CodeGenerator *generator) {
	size_t label;

	label = generator->next_label;
	generator->next_label++;

	return label;
}

static void codegen_emit_label_reference(const CodeGenerator *generator,
					 const size_t label) {
	fprintf(generator->output, "pgc_label_%zu", label);
}

static void codegen_emit_label(const CodeGenerator *generator,
			       const size_t label) {
	codegen_emit_indent(generator);
	codegen_emit_label_reference(generator, label);
	fputs(":;\n", generator->output);
}

static void codegen_emit_jump(const CodeGenerator *generator,
			      const char *instruction,
			      const size_t label) {
	codegen_emit_indent(generator);
	fprintf(generator->output, "%s(", instruction);
	codegen_emit_label_reference(generator, label);
	fputs(");\n", generator->output);
}

static bool codegen_push_loop(CodeGenerator *generator,
			      const size_t continue_label,
			      const size_t break_label,
			      const size_t line) {
	CodegenLoop *loop;

	if (generator->loop_count >= CODEGEN_MAX_LOOP_DEPTH) {
		codegen_error(generator, line, "loop nesting limit exceeded");
		return false;
	}

	loop = &generator->loops[generator->loop_count];
	loop->continue_label = continue_label;
	loop->break_label = break_label;
	generator->loop_count++;

	return true;
}

static void codegen_pop_loop(CodeGenerator *generator) {
	if (generator->loop_count > 0) { generator->loop_count--; }
}

static const CodegenLoop *codegen_current_loop(const CodeGenerator *generator) {
	if (generator->loop_count == 0) { return NULL; }

	return &generator->loops[generator->loop_count - 1];
}

static void codegen_emit_expression(CodeGenerator *generator,
				    const Expression *expression) {
	size_t i;

	if (expression == NULL) { return; }

	switch (expression->type) {
	case EXPRESSION_IDENTIFIER:
	case EXPRESSION_INTEGER:
	case EXPRESSION_BOOLEAN:
	case EXPRESSION_STRING:
		codegen_emit_token(generator, &expression->token);
		break;

	case EXPRESSION_CALL:
		codegen_emit_expression(generator, expression->call.callee);

		fputc('(', generator->output);

		for (i = 0; i < expression->call.argument_count; i++) {
			codegen_emit_expression(generator,
						expression->call.arguments[i]);

			if (i + 1 < expression->call.argument_count) {
				fputs(", ", generator->output);
			}
		}

		fputc(')', generator->output);
		break;

	case EXPRESSION_UNARY:
		fputc('(', generator->output);
		codegen_emit_token(generator, &expression->unary.operator);
		codegen_emit_expression(generator, expression->unary.operand);
		fputc(')', generator->output);
		break;

	case EXPRESSION_BINARY:
		fputc('(', generator->output);
		codegen_emit_expression(generator, expression->binary.left);
		fputc(' ', generator->output);
		codegen_emit_token(generator, &expression->binary.operator);
		fputc(' ', generator->output);
		codegen_emit_expression(generator, expression->binary.right);
		fputc(')', generator->output);
		break;

	case EXPRESSION_TERNARY:
		fputc('(', generator->output);
		codegen_emit_expression(generator,
					expression->ternary.condition);
		fputs(" ? ", generator->output);
		codegen_emit_expression(generator,
					expression->ternary.then_expression);
		fputs(" : ", generator->output);
		codegen_emit_expression(generator,
					expression->ternary.else_expression);
		fputc(')', generator->output);
		break;

	case EXPRESSION_ASSIGNMENT:
		fputc('(', generator->output);
		codegen_emit_expression(generator,
					expression->assignment.target);
		fputc(' ', generator->output);
		codegen_emit_token(generator, &expression->assignment.operator);
		fputc(' ', generator->output);
		codegen_emit_expression(generator,
					expression->assignment.value);
		fputc(')', generator->output);
		break;

	case EXPRESSION_POSTFIX:
		fputc('(', generator->output);
		codegen_emit_expression(generator, expression->postfix.operand);
		codegen_emit_token(generator, &expression->postfix.operator);
		fputc(')', generator->output);
		break;
	}
}

static void codegen_emit_block(CodeGenerator *generator,
			       const Statement *statement) {
	size_t i;

	fputs("{\n", generator->output);
	generator->indent++;

	for (i = 0; i < statement->block.count; i++) {
		codegen_emit_statement(generator, statement->block.items[i]);
	}

	generator->indent--;
	codegen_emit_indent(generator);
	fputs("}\n", generator->output);
}

static void codegen_emit_if(CodeGenerator *generator,
			    const Statement *statement) {
	size_t false_label;
	size_t end_label;

	false_label = codegen_allocate_label(generator);

	codegen_emit_indent(generator);
	fputs("TEST(", generator->output);
	codegen_emit_expression(generator, statement->if_statement.condition);
	fputs(");\n", generator->output);

	codegen_emit_jump(generator, "JZ", false_label);

	codegen_emit_statement(generator, statement->if_statement.branch);

	if (statement->if_statement.else_branch == NULL) {
		codegen_emit_label(generator, false_label);
		return;
	}

	end_label = codegen_allocate_label(generator);

	codegen_emit_jump(generator, "JMP", end_label);
	codegen_emit_label(generator, false_label);

	codegen_emit_statement(generator, statement->if_statement.else_branch);

	codegen_emit_label(generator, end_label);
}

static void codegen_emit_while(CodeGenerator *generator,
			       const Statement *statement) {
	size_t condition_label;
	size_t end_label;

	condition_label = codegen_allocate_label(generator);
	end_label = codegen_allocate_label(generator);

	codegen_emit_label(generator, condition_label);

	codegen_emit_indent(generator);
	fputs("TEST(", generator->output);
	codegen_emit_expression(generator,
				statement->while_statement.condition);
	fputs(");\n", generator->output);

	codegen_emit_jump(generator, "JZ", end_label);

	if (!codegen_push_loop(generator, condition_label, end_label,
			       statement->line)) {
		return;
	}

	codegen_emit_statement(generator, statement->while_statement.branch);

	codegen_pop_loop(generator);

	codegen_emit_jump(generator, "JMP", condition_label);

	codegen_emit_label(generator, end_label);
}

static void codegen_emit_for_initializer(CodeGenerator *generator,
					 const Statement *statement) {
	switch (statement->for_statement.initializer_type) {
	case FOR_INITIALIZER_NONE: break;

	case FOR_INITIALIZER_EXPRESSION:
		codegen_emit_indent(generator);
		codegen_emit_expression(generator,
					statement->for_statement.initializer);
		fputs(";\n", generator->output);
		break;

	case FOR_INITIALIZER_VARIABLE:
		codegen_emit_indent(generator);

		codegen_emit_token(generator,
				   &statement->for_statement.variable_type);
		fputc(' ', generator->output);
		codegen_emit_token(generator,
				   &statement->for_statement.variable_name);

		if (statement->for_statement.initializer != NULL) {
			fputs(" = ", generator->output);
			codegen_emit_expression(
				generator,
				statement->for_statement.initializer);
		}

		fputs(";\n", generator->output);
		break;
	}
}

static void codegen_emit_for(CodeGenerator *generator,
			     const Statement *statement) {
	size_t condition_label;
	size_t update_label;
	size_t end_label;

	condition_label = codegen_allocate_label(generator);
	update_label = codegen_allocate_label(generator);
	end_label = codegen_allocate_label(generator);

	codegen_emit_indent(generator);
	fputs("{\n", generator->output);
	generator->indent++;

	codegen_emit_for_initializer(generator, statement);
	codegen_emit_label(generator, condition_label);

	if (statement->for_statement.condition != NULL) {
		codegen_emit_indent(generator);
		fputs("TEST(", generator->output);
		codegen_emit_expression(generator,
					statement->for_statement.condition);
		fputs(");\n", generator->output);

		codegen_emit_jump(generator, "JZ", end_label);
	}

	if (!codegen_push_loop(generator, update_label, end_label,
			       statement->line)) {
		generator->indent--;
		return;
	}

	codegen_emit_statement(generator, statement->for_statement.branch);

	codegen_pop_loop(generator);

	codegen_emit_label(generator, update_label);

	if (statement->for_statement.update != NULL) {
		codegen_emit_indent(generator);
		codegen_emit_expression(generator,
					statement->for_statement.update);
		fputs(";\n", generator->output);
	}

	codegen_emit_jump(generator, "JMP", condition_label);

	codegen_emit_label(generator, end_label);

	generator->indent--;
	codegen_emit_indent(generator);
	fputs("}\n", generator->output);
}

static void codegen_emit_statement(CodeGenerator *generator,
				   const Statement *statement) {
	const CodegenLoop *loop;

	if (statement == NULL || generator->had_error) { return; }

	switch (statement->type) {
	case STATEMENT_EXPRESSION:
		codegen_emit_indent(generator);
		codegen_emit_expression(generator, statement->expression.value);
		fputs(";\n", generator->output);
		break;

	case STATEMENT_VARIABLE:
		codegen_emit_indent(generator);
		codegen_emit_token(generator, &statement->variable.type);
		fputc(' ', generator->output);
		codegen_emit_token(generator, &statement->variable.name);

		if (statement->variable.initializer != NULL) {
			fputs(" = ", generator->output);
			codegen_emit_expression(
				generator, statement->variable.initializer);
		}

		fputs(";\n", generator->output);
		break;

	case STATEMENT_RETURN:
		codegen_emit_indent(generator);
		fputs("return", generator->output);

		if (statement->return_statement.value != NULL) {
			fputc(' ', generator->output);
			codegen_emit_expression(
				generator, statement->return_statement.value);
		}

		fputs(";\n", generator->output);
		break;

	case STATEMENT_CONTINUE:
		loop = codegen_current_loop(generator);

		if (loop == NULL) {
			codegen_error(generator, statement->line,
				      "continue used outside loop");
			return;
		}

		codegen_emit_jump(generator, "JMP", loop->continue_label);
		break;

	case STATEMENT_BREAK:
		loop = codegen_current_loop(generator);

		if (loop == NULL) {
			codegen_error(generator, statement->line,
				      "break used outside loop");
			return;
		}

		codegen_emit_jump(generator, "JMP", loop->break_label);
		break;

	case STATEMENT_IF: codegen_emit_if(generator, statement); break;

	case STATEMENT_WHILE: codegen_emit_while(generator, statement); break;

	case STATEMENT_FOR: codegen_emit_for(generator, statement); break;

	case STATEMENT_BLOCK:
		codegen_emit_indent(generator);
		codegen_emit_block(generator, statement);
		break;
	}
}

void code_generator_init(CodeGenerator *generator, FILE *output) {
	generator->output = output;
	generator->indent = 0;
	generator->next_label = 0;
	generator->loop_count = 0;
	generator->had_error = false;
}

void code_generator_emit_preamble(const CodeGenerator *generator) {
	fputs("#include <stdbool.h>\n"
	      "#include <pocketgame.h>\n"
	      "\n",
	      generator->output);
}

bool code_generator_emit_declaration(CodeGenerator *generator,
				     const Declaration *declaration) {
	switch (declaration->type) {
	case DECLARATION_DIRECTIVE:
		codegen_emit_token(generator, &declaration->name);
		fputc('(', generator->output);
		codegen_emit_tokens(generator, declaration->tokens,
				    declaration->token_count);
		fputs(");\n", generator->output);
		break;

	case DECLARATION_FUNCTION_PROTOTYPE:
		codegen_emit_token(generator, &declaration->return_type);
		fputc(' ', generator->output);
		codegen_emit_token(generator, &declaration->name);
		fputc('(', generator->output);

		if (declaration->token_count == 0) {
			fputs("void", generator->output);
		} else {
			codegen_emit_tokens(generator, declaration->tokens,
					    declaration->token_count);
		}

		fputs(");\n", generator->output);
		break;

	case DECLARATION_FUNCTION_DEFINITION:
		codegen_emit_token(generator, &declaration->return_type);
		fputc(' ', generator->output);
		codegen_emit_token(generator, &declaration->name);
		fputc('(', generator->output);

		if (declaration->token_count == 0) {
			fputs("void", generator->output);
		} else {
			codegen_emit_tokens(generator, declaration->tokens,
					    declaration->token_count);
		}

		fputs(") ", generator->output);

		codegen_emit_statement(generator, declaration->body);

		fputc('\n', generator->output);
		break;
	}

	return !generator->had_error && !ferror(generator->output);
}