#include "parser.h"
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

	if (fclose(file) != 0) {
		perror(path);
		free(buffer);
		return NULL;
	}

	return buffer;
}

static const char *declaration_type_name(DeclarationType type) {
	switch (type) {
	case DECLARATION_DIRECTIVE: return "directive";
	case DECLARATION_FUNCTION_PROTOTYPE: return "function prototype";
	case DECLARATION_FUNCTION_DEFINITION: return "function definition";
	}

	return "unknown";
}

static void print_declaration(const Declaration *declaration) {
	printf("%zu: %-20s %.*s", declaration->line,
	       declaration_type_name(declaration->type),
	       (int)declaration->name.length, declaration->name.start);

	if (declaration->type == DECLARATION_FUNCTION_PROTOTYPE ||
	    declaration->type == DECLARATION_FUNCTION_DEFINITION) {
		printf(" -> %.*s", (int)declaration->return_type.length,
		       declaration->return_type.start);
	}

	putchar('\n');
}

int main(int argc, char **argv) {
	const char *input_path;
	char *source;
	Parser parser;
	Declaration declaration;

	if (argc != 2) {
		fprintf(stderr, "usage: pcc <input-file>\n");
		return EXIT_FAILURE;
	}

	input_path = argv[1];
	source = read_file(input_path);

	if (source == NULL) { return EXIT_FAILURE; }

	parser_init(&parser, input_path, source);

	while (parser_next_declaration(&parser, &declaration)) {
		print_declaration(&declaration);
		declaration_destroy(&declaration);
	}

	free(source);

	if (parser.had_error) { return EXIT_FAILURE; }

	return EXIT_SUCCESS;
}