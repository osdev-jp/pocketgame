CC		:= gcc
AR		:= ar
SDL_CFLAGS	:= $(shell sdl2-config --cflags)
SDL_LIBS	:= $(shell sdl2-config --libs)
CFLAGS		:= -Wall -Wextra -Werror -std=c11 -Iinclude $(SDL_CFLAGS)

OUT		:= out
TARGET		:= $(OUT)/libpocketgame.a

SRC		:= $(shell find src -name '*.c')
LIB_SRC		:= $(filter-out src/pocketgame_main.c,$(SRC))
HEADER		:= $(shell find include -name '*.h')
OBJ		:= $(patsubst src/%.c,$(OUT)/%.o,$(LIB_SRC))

EXAMPLE		?= examples/osero.c

ifeq ($(OS),Windows_NT)
PLATFORM	:= windows
PREFIX		?= C:/Program Files/PocketGameLibrary
else
PLATFORM	:= linux
PREFIX		?= /usr/local
endif

LIBDIR		:= $(PREFIX)/lib
INCLUDEDIR	:= $(PREFIX)/include

.PHONY: all run clean install fmt

all: $(TARGET) fmt

$(TARGET): $(OBJ)
	mkdir -p "$(OUT)"
	$(AR) rcs "$@" $^

$(OUT)/%.o: src/%.c
	mkdir -p "$(dir $@)"
	$(CC) $(CFLAGS) -c "$<" -o "$@"

install: $(TARGET)
	sudo mkdir -p "$(LIBDIR)"
	sudo mkdir -p "$(INCLUDEDIR)"
	sudo cp "$(TARGET)" "$(LIBDIR)/"
	sudo cp -r include/. "$(INCLUDEDIR)/"

run: osero

osero:
	@echo "Running osero example..."
	@$(CC) $(CFLAGS) examples/osero.c src/pocketgame_main.c -o "$(OUT)/osero" -L"$(OUT)" -lpocketgame $(SDL_LIBS)
	@"$(OUT)/osero"

gomoku:
	@echo "Running gomoku example..."
	@$(CC) $(CFLAGS) examples/gomoku.c src/pocketgame_main.c -o "$(OUT)/gomoku" -L"$(OUT)" -lpocketgame $(SDL_LIBS)
	@"$(OUT)/gomoku"

fmt:
	@echo "Formatting..."
	@clang-format -i $(SRC) $(HEADER) $(EXAMPLE)

clean:
	rm -rf "$(OUT)"
