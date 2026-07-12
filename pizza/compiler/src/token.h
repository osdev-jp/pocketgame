#ifndef POCKETGAME_TOKEN_H
#define POCKETGAME_TOKEN_H

#include <stddef.h>

typedef enum {
	TOKEN_EOF,
	TOKEN_IDENTIFIER,
	TOKEN_INTEGER,
	TOKEN_STRING,
	TOKEN_LEFT_PAREN,
	TOKEN_RIGHT_PAREN,
	TOKEN_LEFT_BRACE,
	TOKEN_RIGHT_BRACE,
	TOKEN_COMMA,
	TOKEN_SEMICOLON,
	TOKEN_HASH,
	TOKEN_INVALID
} TokenType;

typedef struct {
	TokenType type;
	const char *start;
	size_t length;
	size_t line;
	size_t column;
} Token;

const char *token_type_name(TokenType type);

#endif