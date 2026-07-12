CC	:= cc
CFLAGS	:= -std=c11 -Wall -Wextra -Werror -Iinclude
OUT	:= out
TARGET	:= $(OUT)/pcc
SRC	:= $(shell find src -name '*.c')

OBJ	:= $(patsubst src/%.c,$(OUT)/%.o,$(SRC))

.PHONY: all clean fmt run

all: $(TARGET)

$(TARGET): $(OBJ)
	@$(CC) $(CFLAGS) -o $@ $(OBJ)

$(OUT)/%.o: src/%.c
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo "Cleaning up: $(OUT)"
	@rm -rf $(OUT)

run: $(TARGET)
	@./$(TARGET) $(shell pwd)/examples/osero.c

fmt:
	@echo "Formatting..."
	@clang-format -i $(SRC) src/*.c src/*.h examples/*.c