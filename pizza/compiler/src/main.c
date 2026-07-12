#include "lexer.h"
#include "token.h"
#include <stdio.h>
#include <stdlib.h>

static char *read_file(const char *path) {
	FILE *file;
	char *buffer;
	long size;
	size_t read_size;

	file = fopen(path, "rb");

	if (file == NULL) {
		perror(path);
		return NULL;
	}

	if (fseek(file, 0, SEEK_END) != 0) {
		perror(path);
		fclose(file);
		return NULL;
	}

	size = ftell(file);

	if (size < 0) {
		perror(path);
		fclose(file);
		return NULL;
	}

	if (fseek(file, 0, SEEK_SET) != 0) {
		perror(path);
		fclose(file);
		return NULL;
	}

	buffer = malloc((size_t)size + 1);

	if (buffer == NULL) {
		fprintf(stderr, "failed to allocate source buffer\n");
		fclose(file);
		return NULL;
	}

	read_size = fread(buffer, 1, (size_t)size, file);

	if (read_size != (size_t)size) {
		fprintf(stderr, "failed to read source file\n");
		free(buffer);
		fclose(file);
		return NULL;
	}

	buffer[size] = '\0';
	fclose(file);

	return buffer;
}

static void print_token(const Token *token) {
	printf("%zu:%zu %-16s", token->line, token->column,
	       token_type_name(token->type));

	if (token->length > 0) {
		printf(" \"%.*s\"", (int)token->length, token->start);
	}

	putchar('\n');
}

int main(int argc, char **argv) {
	const char *input_path;
	char *source;
	Lexer lexer;
	Token token;

	if (argc != 2) {
		fprintf(stderr, "usage: pcc <input-file>\n");
		return EXIT_FAILURE;
	}

	input_path = argv[1];
	source = read_file(input_path);

	if (source == NULL) { return EXIT_FAILURE; }

	lexer_init(&lexer, source);

	do {
		token = lexer_next_token(&lexer);
		print_token(&token);

		if (token.type == TOKEN_INVALID) {
			fprintf(stderr, "%s:%zu:%zu: invalid token\n",
				input_path, token.line, token.column);

			free(source);
			return EXIT_FAILURE;
		}
	} while (token.type != TOKEN_EOF);

	free(source);
	return EXIT_SUCCESS;
}