CC	:= cc
CFLAGS	:= -std=c11 -Wall -Wextra -Werror -Iinclude
OUT	:= out
TARGET	:= $(OUT)/pgcc
SRC	:= $(shell find src -name '*.c')

OBJ	:= $(patsubst src/%.c,$(OUT)/%.o,$(SRC))

.PHONY: all clean fmt

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

$(OUT)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OUT)

fmt:
	clang-format -i $(SRC) src/*.c src/*.h