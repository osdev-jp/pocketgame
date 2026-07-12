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

POCKETGAME_LIB	:= $(TARGET)

PCC_DIR		:= $(abspath ../compiler)
PCC		:= $(PCC_DIR)/out/pcc
PCC_SOURCE	:= $(PCC_DIR)/examples/osero.c
PCC_OUT		:= $(PCC_DIR)/examples/osero.pas
PCC_TARGET	:= $(OUT)/osero-pas
PCC_LIB_DIR	:= $(abspath ./include)
PCC_SRC_DIR := $(abspath ./src)
PCC_HEADER	:= $(PCC_LIB_DIR)/pocketgame.h
PCC_RUNTIME	:= $(PCC_SRC_DIR)/pocketgame.c

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

.PHONY: all run pas clean install fmt osero gomoku

all: $(TARGET)

$(TARGET): $(OBJ)
	mkdir -p "$(OUT)"
	$(AR) rcs "$@" $^

$(OUT)/%.o: src/%.c
	mkdir -p "$(dir $@)"
	$(CC) $(CFLAGS) -c "$<" -o "$@"

run: osero

pas: $(POCKETGAME_LIB)
	@echo "Building pcc..."
	@$(MAKE) -C "$(PCC_DIR)"
	@echo "Compiling $(PCC_SOURCE)..."
	@"$(PCC)" -S "$(PCC_SOURCE)" -o "$(PCC_OUT)"
	@echo "Building generated PocketGame code..."
	@mkdir -p "$(OUT)"
	@$(CC) $(CFLAGS) \
		-Wno-unused-label \
		-I"$(PCC_LIB_DIR)" \
		-include pocketgame.h \
		-x c "$(PCC_OUT)" \
		-x none "$(PCC_RUNTIME)" \
		$(POCKETGAME_LIB) \
		$(SDL_LIBS) \
		-o "$(PCC_TARGET)"
	@"$(PCC_TARGET)"

osero: $(POCKETGAME_LIB)
	@echo "Running osero example..."
	@$(CC) $(CFLAGS) \
		examples/osero.c \
		src/pocketgame_main.c \
		$(POCKETGAME_LIB) \
		$(SDL_LIBS) \
		-o "$(OUT)/osero"
	@"$(OUT)/osero"

gomoku: $(POCKETGAME_LIB)
	@echo "Running gomoku example..."
	@$(CC) $(CFLAGS) \
		examples/gomoku.c \
		src/pocketgame_main.c \
		$(POCKETGAME_LIB) \
		$(SDL_LIBS) \
		-o "$(OUT)/gomoku"
	@"$(OUT)/gomoku"

fmt:
	@echo "Formatting..."
	@clang-format -i $(SRC) $(HEADER) $(EXAMPLE)

clean:
	rm -rf "$(OUT)"
