#ifndef POCKETGAME_ASM_H
#define POCKETGAME_ASM_H

#include <stdbool.h>

#include "graphics.h"

extern PgRoutine __pg_on_start;
extern PgRoutine __pg_on_frame;
extern bool __pg_test_result;

#define GAME(name) \
  const char* pg_game_name(void) { return #name; }

#define MESSAGE(name, text) const char* name = (text)

#define CURSOR(name, grid_name) PgCursor name = {0, 0, &(grid_name)}

#define GRID(name, w, h) PgGrid name = {(w), (h), 0, 0}

#define SELECT(grid, cursor) ((grid).cursor = &(cursor))

#define STATE(name, value) int name = (value)

#define ON_START(fn) PgRoutine __pg_on_start = (fn)

#define ON_FRAME(fn) PgRoutine __pg_on_frame = (fn)

#define SET_CELL(grid, x, y, value) pg_grid_set(&(grid), (x), (y), (value))

#define SET_CELL_AT(grid, cursor, value) \
  pg_grid_set_at(&(grid), &(cursor), (value))

#define GET_CELL(dst, grid, x, y) ((dst) = pg_grid_get(&(grid), (x), (y)))

#define CELL_AT(dst, grid, cursor) ((dst) = pg_grid_get_at(&(grid), &(cursor)))

#define GET_CURSOR_POSITION(x, y, cursor) \
  do {                                    \
    (x) = (cursor).x;                     \
    (y) = (cursor).y;                     \
  } while (0)

#define READ_INPUT(dst) ((dst) = pg_read_input())

#define PRESSED(dst, input, button) \
  ((dst) = pg_input_pressed((input), (button)))

#define IF(cond, fn)  \
  do {                \
    if (cond) (fn)(); \
  } while (0)

#define MOVE(target, dx, dy) pg_cursor_move(&(target), (dx), (dy))

#define CLAMP_TO_GRID(target) pg_cursor_clamp(&(target))

#define SET(dst, value) ((dst) = (value))

#define ADD(dst, value) ((dst) += (value))

#define SUB(dst, value) ((dst) -= (value))

#define SWAP(a, b) pg_swap_int(&(a), &(b))

#define TEST(value) (__pg_test_result = !!(value))

#define JZ(label)                      \
  do {                                 \
    if (!__pg_test_result) goto label; \
  } while (0)

#define JNZ(label)                    \
  do {                                \
    if (__pg_test_result) goto label; \
  } while (0)

#define JMP(label) goto label

#define RUN() pg_run(__pg_on_start, __pg_on_frame)

#define POCKETGAME_MAIN() \
  int main(void) { return RUN(); }

#define INPUT(name) PgInput name = {PG_BUTTON_NONE}

#define FLAG(name) bool name = false

#define GRAPHIC() pg_graphic_open(WIDTH, HEIGHT)

#define DRAW(target) pg_graphic_draw_grid(&(target))

#define SHOW_MESSAGE(message) pg_graphic_show_message((message))

#endif /* POCKETGAME_ASM_H */
