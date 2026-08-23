ARCH			?= x86_64
ARCHES			:= x86_64 pg_rv32
X86_64_CC		?= gcc
X86_64_AR		?= ar
PG_RV32_CC		?= riscv32-unknown-elf-gcc
PG_RV32_AR		?= riscv32-unknown-elf-ar
PG_RV32_CFLAGS		?=
OUT			:= target
BIN			:= libpocketgame.a
TARGET			:= $(OUT)/$(ARCH)/$(BIN)
OBJ_DIR			:= $(OUT)/$(ARCH)/obj

EXAMPLE			?= examples/osero.c

ifeq ($(filter $(ARCH),$(ARCHES)),)
$(error Unsupported architecture "$(ARCH)". Supported architectures: $(ARCHES))
endif

CFLAGS			:= -Wall -Wextra -Werror -std=c11 -Iinclude
CFLAGS			+= -MMD -MP

ifeq ($(ARCH),x86_64)
CC			:= $(X86_64_CC)
AR			:= $(X86_64_AR)
SDL_CFLAGS		:= $(shell sdl2-config --cflags)
CFLAGS			+= $(SDL_CFLAGS)
else ifeq ($(ARCH),pg_rv32)
CC			:= $(PG_RV32_CC)
AR			:= $(PG_RV32_AR)
CFLAGS			+= $(PG_RV32_CFLAGS)
endif

SRC			:= $(shell find src -name '*.c')
COMMON_SRC		:= $(filter-out src/pocketgame_main.c src/arch/%,$(SRC))
ARCH_SRC		:= $(shell find src/arch/$(ARCH) -name '*.c')
LIB_SRC			:= $(COMMON_SRC) $(ARCH_SRC)
HEADER			:= $(shell find include -name '*.h')
OBJ			:= $(patsubst src/%.c,$(OBJ_DIR)/%.o,$(LIB_SRC))
DEP			:= $(OBJ:.o=.d)

.PHONY: all run osero gomoku fmt clean x86_64 pg_rv32

all: $(TARGET)

$(TARGET): $(OBJ)
	@mkdir -p "$(dir $@)"
	@rm -f "$@"
	$(AR) rcs "$@" $^

$(OBJ_DIR)/%.o: src/%.c
	@mkdir -p "$(dir $@)"
	$(CC) $(CFLAGS) -c "$<" -o "$@"

x86_64:
	$(MAKE) ARCH=x86_64 all

pg_rv32:
	$(MAKE) ARCH=pg_rv32 all

run: osero

osero:
	@./emulator.pl --arch x86_64 examples/osero.c

gomoku:
	@./emulator.pl --arch x86_64 examples/gomoku.c

fmt:
	@echo "Formatting..."
	@clang-format -i $(SRC) $(HEADER) $(EXAMPLE)

clean:
	rm -rf "$(OUT)"

-include $(DEP)
