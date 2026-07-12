#ifndef POCKETGAME_CODEGEN_H
#define POCKETGAME_CODEGEN_H
#include "ast.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#define CODEGEN_MAX_LOOP_DEPTH 128

typedef struct {
	size_t continue_label;
	size_t break_label;
} CodegenLoop;

typedef struct {
	FILE *output;
	size_t indent;
	size_t next_label;
	CodegenLoop loops[CODEGEN_MAX_LOOP_DEPTH];
	size_t loop_count;
	bool had_error;
} CodeGenerator;

void code_generator_init(CodeGenerator *generator, FILE *output);
void code_generator_emit_preamble(const CodeGenerator *generator);
bool code_generator_emit_declaration(CodeGenerator *generator,
				     const Declaration *declaration);

#endif