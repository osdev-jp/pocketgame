#include "codegen.h"
#include "parser.h"
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	const char *input_path;
	const char *output_path;
	bool emit_assembly;
} CommandLineOptions;

static void print_usage(FILE *stream, const char *program) {
	fprintf(stream,
		"Usage: %s [options] file\n"
		"\n"
		"Options:\n"
		"  -S              Generate C-compatible PocketGame assembly\n"
		"  -o <file>       Write output to <file>\n"
		"  -o<file>        Write output to <file>\n"
		"  -h, --help      Display this help\n"
		"  --version       Display compiler version\n"
		"  --              End option parsing\n",
		program);
}

static int parse_arguments(int argc, char **argv, CommandLineOptions *options) {
	bool options_enabled;
	int i;

	options->input_path = NULL;
	options->output_path = NULL;
	options->emit_assembly = false;
	options_enabled = true;

	for (i = 1; i < argc; i++) {
		const char *argument;

		argument = argv[i];

		if (options_enabled && strcmp(argument, "--") == 0) {
			options_enabled = false;
			continue;
		}

		if (options_enabled && (strcmp(argument, "-h") == 0 ||
					strcmp(argument, "--help") == 0)) {
			print_usage(stdout, argv[0]);
			return 1;
		}

		if (options_enabled && strcmp(argument, "--version") == 0) {
			puts("pcc - PocketGame Compiler");
			puts(VERSION);
			return 1;
		}

		if (options_enabled && strcmp(argument, "-S") == 0) {
			options->emit_assembly = true;
			continue;
		}

		if (options_enabled && strcmp(argument, "-o") == 0) {
			if (i + 1 >= argc) {
				fprintf(stderr,
					"%s: error: missing filename after "
					"'-o'\n",
					argv[0]);
				return -1;
			}

			i++;
			options->output_path = argv[i];
			continue;
		}

		if (options_enabled && strncmp(argument, "-o", 2) == 0 &&
		    argument[2] != '\0') {
			options->output_path = argument + 2;
			continue;
		}

		if (options_enabled && argument[0] == '-' &&
		    argument[1] != '\0' && strcmp(argument, "-") != 0) {
			fprintf(stderr, "%s: error: unrecognized option '%s'\n",
				argv[0], argument);
			return -1;
		}

		if (options->input_path != NULL) {
			fprintf(stderr,
				"%s: error: multiple input files are not yet "
				"supported\n",
				argv[0]);
			return -1;
		}

		options->input_path = argument;
	}

	if (options->input_path == NULL) {
		fprintf(stderr, "%s: fatal error: no input files\n", argv[0]);
		return -1;
	}

	if (options->output_path != NULL &&
	    strcmp(options->input_path, "-") != 0 &&
	    strcmp(options->output_path, "-") != 0 &&
	    strcmp(options->input_path, options->output_path) == 0) {
		fprintf(stderr,
			"%s: fatal error: input file is the same as output "
			"file\n",
			argv[0]);
		return -1;
	}

	return 0;
}

static char *read_stream(FILE *stream, const char *display_path) {
	char *source;
	size_t capacity;
	size_t length;

	capacity = 4096;
	length = 0;
	source = malloc(capacity);

	if (source == NULL) {
		fprintf(stderr, "%s: failed to allocate source buffer\n",
			display_path);
		return NULL;
	}

	for (;;) {
		size_t available;
		size_t read_size;

		if (length + 1 >= capacity) {
			char *new_source;
			size_t new_capacity;

			new_capacity = capacity * 2;
			new_source = realloc(source, new_capacity);

			if (new_source == NULL) {
				fprintf(stderr,
					"%s: failed to grow source buffer\n",
					display_path);

				free(source);
				return NULL;
			}

			source = new_source;
			capacity = new_capacity;
		}

		available = capacity - length - 1;
		read_size = fread(source + length, 1, available, stream);

		length += read_size;

		if (read_size < available) {
			if (ferror(stream)) {
				fprintf(stderr,
					"%s: failed to read input: %s\n",
					display_path, strerror(errno));

				free(source);
				return NULL;
			}

			break;
		}
	}

	source[length] = '\0';
	return source;
}

static char *read_source(const char *input_path) {
	FILE *input;
	char *source;

	if (strcmp(input_path, "-") == 0) {
		return read_stream(stdin, "<stdin>");
	}

	input = fopen(input_path, "rb");

	if (input == NULL) {
		fprintf(stderr, "%s: failed to open input: %s\n", input_path,
			strerror(errno));
		return NULL;
	}

	source = read_stream(input, input_path);

	if (fclose(input) != 0) {
		fprintf(stderr, "%s: failed to close input: %s\n", input_path,
			strerror(errno));

		free(source);
		return NULL;
	}

	return source;
}

static FILE *open_output(const char *output_path) {
	FILE *output;

	if (output_path == NULL || strcmp(output_path, "-") == 0) {
		return stdout;
	}

	output = fopen(output_path, "wb");

	if (output == NULL) {
		fprintf(stderr, "%s: failed to open output: %s\n", output_path,
			strerror(errno));
		return NULL;
	}

	return output;
}

int main(int argc, char **argv) {
	CommandLineOptions options;
	CodeGenerator generator;
	Declaration declaration;
	Parser parser;
	const char *parser_input_path;
	char *source;
	FILE *output;
	bool success;
	int parse_result;

	parse_result = parse_arguments(argc, argv, &options);

	if (parse_result > 0) { return 0; }

	if (parse_result < 0) {
		print_usage(stderr, argv[0]);
		return 1;
	}

	source = read_source(options.input_path);

	if (source == NULL) { return 1; }

	output = open_output(options.output_path);

	if (output == NULL) {
		free(source);
		return 1;
	}

	parser_input_path = strcmp(options.input_path, "-") == 0
				    ? "<stdin>"
				    : options.input_path;

	parser_init(&parser, parser_input_path, source);

	code_generator_init(&generator, output);
	code_generator_emit_preamble(&generator);

	success = true;

	while (parser_next_declaration(&parser, &declaration)) {
		if (!code_generator_emit_declaration(&generator,
						     &declaration)) {
			success = false;
		}

		declaration_destroy(&declaration);

		if (!success) { break; }
	}

	if (parser.had_error || generator.had_error) { success = false; }

	if (output != stdout && fclose(output) != 0) {
		fprintf(stderr, "%s: failed to close output: %s\n",
			options.output_path, strerror(errno));
		success = false;
	}

	if (!success && options.output_path != NULL &&
	    strcmp(options.output_path, "-") != 0) {
		remove(options.output_path);
	}

	free(source);

	return success ? 0 : 1;
}