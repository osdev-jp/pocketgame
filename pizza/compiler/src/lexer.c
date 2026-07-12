#include "lexer.h"
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>

static bool lexer_is_at_end(const Lexer *lexer) {
	return *lexer->current == '\0';
}

static char lexer_peek(const Lexer *lexer) { return *lexer->current; }

static char lexer_advance(Lexer *lexer) {
	char character;

	character = *lexer->current;

	if (character == '\0') { return '\0'; }

	lexer->current++;

	if (character == '\n') {
		lexer->line++;
		lexer->column = 1;
	} else {
		lexer->column++;
	}

	return character;
}

static void lexer_skip_whitespace(Lexer *lexer) {
	while (!lexer_is_at_end(lexer)) {
		switch (lexer_peek(lexer)) {
		case ' ':
		case '\t':
		case '\r':
		case '\n': lexer_advance(lexer); break;
		default: return;
		}
	}
}

static Token lexer_make_token(TokenType type,
			      const char *start,
			      size_t length,
			      size_t line,
			      size_t column) {
	Token token;

	token.type = type;
	token.start = start;
	token.length = length;
	token.line = line;
	token.column = column;

	return token;
}

static Token lexer_identifier(Lexer *lexer) {
	const char *start;
	size_t line;
	size_t column;

	start = lexer->current;
	line = lexer->line;
	column = lexer->column;

	while (isalnum((unsigned char)lexer_peek(lexer)) ||
	       lexer_peek(lexer) == '_') {
		lexer_advance(lexer);
	}

	return lexer_make_token(TOKEN_IDENTIFIER, start,
				(size_t)(lexer->current - start), line, column);
}

static Token lexer_integer(Lexer *lexer) {
	const char *start;
	size_t line;
	size_t column;

	start = lexer->current;
	line = lexer->line;
	column = lexer->column;

	while (isdigit((unsigned char)lexer_peek(lexer))) {
		lexer_advance(lexer);
	}

	return lexer_make_token(TOKEN_INTEGER, start,
				(size_t)(lexer->current - start), line, column);
}

static Token lexer_string(Lexer *lexer) {
	const char *start;
	size_t line;
	size_t column;

	line = lexer->line;
	column = lexer->column;

	lexer_advance(lexer);
	start = lexer->current;

	while (!lexer_is_at_end(lexer) && lexer_peek(lexer) != '"') {
		lexer_advance(lexer);
	}

	if (lexer_is_at_end(lexer)) {
		return lexer_make_token(TOKEN_INVALID, start,
					(size_t)(lexer->current - start), line,
					column);
	}

	lexer_advance(lexer);

	return lexer_make_token(TOKEN_STRING, start,
				(size_t)(lexer->current - start - 1), line,
				column);
}

void lexer_init(Lexer *lexer, const char *source) {
	lexer->source = source;
	lexer->current = source;
	lexer->line = 1;
	lexer->column = 1;
}

Token lexer_next_token(Lexer *lexer) {
	const char *start;
	size_t line;
	size_t column;
	char character;

	lexer_skip_whitespace(lexer);

	start = lexer->current;
	line = lexer->line;
	column = lexer->column;

	if (lexer_is_at_end(lexer)) {
		return lexer_make_token(TOKEN_EOF, start, 0, line, column);
	}

	character = lexer_peek(lexer);

	if (isalpha((unsigned char)character) || character == '_') {
		return lexer_identifier(lexer);
	}

	if (isdigit((unsigned char)character)) { return lexer_integer(lexer); }

	if (character == '"') { return lexer_string(lexer); }

	lexer_advance(lexer);

	switch (character) {
	case '(':
		return lexer_make_token(TOKEN_LEFT_PAREN, start, 1, line,
					column);
	case ')':
		return lexer_make_token(TOKEN_RIGHT_PAREN, start, 1, line,
					column);
	case '{':
		return lexer_make_token(TOKEN_LEFT_BRACE, start, 1, line,
					column);
	case '}':
		return lexer_make_token(TOKEN_RIGHT_BRACE, start, 1, line,
					column);
	case ',': return lexer_make_token(TOKEN_COMMA, start, 1, line, column);
	case ';':
		return lexer_make_token(TOKEN_SEMICOLON, start, 1, line,
					column);
	case '#': return lexer_make_token(TOKEN_HASH, start, 1, line, column);
	default: return lexer_make_token(TOKEN_INVALID, start, 1, line, column);
	}
}