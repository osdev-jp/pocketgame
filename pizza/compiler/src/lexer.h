#ifndef POCKETGAME_LEXER_H
#define POCKETGAME_LEXER_H

#include "token.h"
#include <stddef.h>

typedef struct {
	const char *source;
	const char *current;
	size_t line;
	size_t column;
} Lexer;

void lexer_init(Lexer *lexer, const char *source);
Token lexer_next_token(Lexer *lexer);

#endif