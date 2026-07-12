#ifndef POCKETGAME_PARSER_H
#define POCKETGAME_PARSER_H

#include "ast.h"
#include "lexer.h"
#include "token.h"
#include <stdbool.h>

typedef enum {
	DECLARATION_DIRECTIVE,
	DECLARATION_FUNCTION_PROTOTYPE,
	DECLARATION_FUNCTION_DEFINITION
} DeclarationType;

typedef struct {
	DeclarationType type;
	Token name;
	Token return_type;
	size_t line;
	Statement *body;
} Declaration;

typedef struct {
	Lexer lexer;
	Token current;
	Token previous;
	const char *input_path;
	bool had_error;
} Parser;

void parser_init(Parser *parser, const char *input_path, const char *source);
bool parser_next_declaration(Parser *parser, Declaration *declaration);
void declaration_destroy(Declaration *declaration);

#endif