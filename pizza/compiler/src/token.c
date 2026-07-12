#include "token.h"

const char *token_type_name(TokenType type) {
	switch (type) {
	case TOKEN_EOF: return "EOF";
	case TOKEN_IDENTIFIER: return "IDENTIFIER";
	case TOKEN_INTEGER: return "INTEGER";
	case TOKEN_STRING: return "STRING";
	case TOKEN_LEFT_PAREN: return "LEFT_PAREN";
	case TOKEN_RIGHT_PAREN: return "RIGHT_PAREN";
	case TOKEN_LEFT_BRACE: return "LEFT_BRACE";
	case TOKEN_RIGHT_BRACE: return "RIGHT_BRACE";
	case TOKEN_COMMA: return "COMMA";
	case TOKEN_SEMICOLON: return "SEMICOLON";
	case TOKEN_HASH: return "HASH";
	case TOKEN_INVALID: return "INVALID";
	}

	return "UNKNOWN";
}