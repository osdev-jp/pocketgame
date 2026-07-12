CC		:= gcc
AR		:= ar
SDL_CFLAGS	:= $(shell sdl2-config --cflags)
CFLAGS		:= -Wall -Wextra -Werror -std=c11 -Iinclude $(SDL_CFLAGS)

OUT		:= out
TARGET		:= $(OUT)/libpocketgame.a

SRC		:= $(shell find src -name '*.c')
LIB_SRC		:= $(filter-out src/pocketgame_main.c,$(SRC))
HEADER		:= $(shell find include -name '*.h')
OBJ		:= $(patsubst src/%.c,$(OUT)/%.o,$(LIB_SRC))

EXAMPLE		?= examples/osero.c

.PHONY: all run osero gomoku fmt clean

all: $(TARGET)

$(TARGET): $(OBJ)
	@mkdir -p "$(OUT)"
	$(AR) rcs "$@" $^

$(OUT)/%.o: src/%.c
	@mkdir -p "$(dir $@)"
	$(CC) $(CFLAGS) -c "$<" -o "$@"

run: osero

osero: all
	@./emulator.pl examples/osero.c

gomoku: all
	@./emulator.pl examples/gomoku.c

fmt:
	@echo "Formatting..."
	@clang-format -i $(SRC) $(HEADER) $(EXAMPLE)

clean:
	rm -rf "$(OUT)"
