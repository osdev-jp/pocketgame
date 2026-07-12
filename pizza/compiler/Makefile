CC	:= cc
CFLAGS	:= -std=c11 -Wall -Wextra -Werror -Iinclude
OUT	:= out
TARGET	:= $(OUT)/pcc
CONFIG	:= .config
SRC	:= $(shell find src -name '*.c')
OBJ	:= $(patsubst src/%.c,$(OUT)/%.o,$(SRC))

ifeq ($(wildcard $(CONFIG)),)
$(error Config file not found: $(CONFIG))
endif

CONFIG_DEFINES := $(shell awk '\
	function trim(value) { \
		sub(/^[[:space:]]+/, "", value); \
		sub(/[[:space:]]+$$/, "", value); \
		return value; \
	} \
	function shell_quote(value, quote, result, i, character) { \
		quote = sprintf("%c", 39); \
		result = quote; \
		for (i = 1; i <= length(value); i++) { \
			character = substr(value, i, 1); \
			if (character == quote) \
				result = result quote "\\" quote quote; \
			else \
				result = result character; \
		} \
		return result quote; \
	} \
	{ \
		line = trim($$0); \
		if (line == "" || substr(line, 1, 1) == "#") \
			next; \
		separator = index(line, ":"); \
		if (separator == 0) { \
			printf "%s:%d: expected KEY: value\n", FILENAME, NR > "/dev/stderr"; \
			exit 1; \
		} \
		key = trim(substr(line, 1, separator - 1)); \
		value = trim(substr(line, separator + 1)); \
		if (key !~ /^[A-Za-z_][A-Za-z0-9_]*$$/) { \
			printf "%s:%d: invalid key: %s\n", FILENAME, NR, key > "/dev/stderr"; \
			exit 1; \
		} \
		if (value == "") { \
			printf "%s:%d: empty value: %s\n", FILENAME, NR, key > "/dev/stderr"; \
			exit 1; \
		} \
		print shell_quote("-D" key "=" value); \
	} \
' $(CONFIG) || printf '__CONFIG_ERROR__')

ifneq ($(findstring __CONFIG_ERROR__,$(CONFIG_DEFINES)),)
$(error Failed to parse $(CONFIG))
endif

CFLAGS	+= $(CONFIG_DEFINES)

.PHONY: all clean fmt run

all: $(TARGET)

$(TARGET): $(OBJ)
	@$(CC) $(CFLAGS) -o $@ $(OBJ)

$(OUT)/%.o: src/%.c $(CONFIG)
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo "Cleaning up: $(OUT)"
	@rm -rf $(OUT)

run: $(TARGET)
	@./$(TARGET) $(shell pwd)/examples/osero.c -o $(shell pwd)/examples/osero.pas

fmt:
	@echo "Formatting..."
	@clang-format -i $(SRC) src/*.h examples/*.c