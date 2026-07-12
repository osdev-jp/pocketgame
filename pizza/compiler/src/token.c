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
	case TOKEN_LEFT_BRACKET: return "LEFT_BRACKET";
	case TOKEN_RIGHT_BRACKET: return "RIGHT_BRACKET";
	case TOKEN_COMMA: return "COMMA";
	case TOKEN_SEMICOLON: return "SEMICOLON";
	case TOKEN_COLON: return "COLON";
	case TOKEN_QUESTION: return "QUESTION";
	case TOKEN_DOT: return "DOT";
	case TOKEN_PLUS: return "PLUS";
	case TOKEN_MINUS: return "MINUS";
	case TOKEN_STAR: return "STAR";
	case TOKEN_SLASH: return "SLASH";
	case TOKEN_PERCENT: return "PERCENT";
	case TOKEN_ASSIGN: return "ASSIGN";
	case TOKEN_EQUAL: return "EQUAL";
	case TOKEN_NOT: return "NOT";
	case TOKEN_NOT_EQUAL: return "NOT_EQUAL";
	case TOKEN_LESS: return "LESS";
	case TOKEN_LESS_EQUAL: return "LESS_EQUAL";
	case TOKEN_GREATER: return "GREATER";
	case TOKEN_GREATER_EQUAL: return "GREATER_EQUAL";
	case TOKEN_AND: return "AND";
	case TOKEN_LOGICAL_AND: return "LOGICAL_AND";
	case TOKEN_OR: return "OR";
	case TOKEN_LOGICAL_OR: return "LOGICAL_OR";
	case TOKEN_PLUS_ASSIGN: return "PLUS_ASSIGN";
	case TOKEN_MINUS_ASSIGN: return "MINUS_ASSIGN";
	case TOKEN_STAR_ASSIGN: return "STAR_ASSIGN";
	case TOKEN_SLASH_ASSIGN: return "SLASH_ASSIGN";
	case TOKEN_INCREMENT: return "INCREMENT";
	case TOKEN_DECREMENT: return "DECREMENT";
	case TOKEN_INVALID: return "INVALID";
	}

	return "UNKNOWN";
}