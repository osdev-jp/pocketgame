#ifndef POCKETGAME_H
#define POCKETGAME_H

#include <stdbool.h>

#include "asm.h"
#include "graphics.h"
#include "types.h"

#define empty PG_TOKEN_EMPTY
#define black PG_TOKEN_BLACK
#define white PG_TOKEN_WHITE
#define red PG_TOKEN_RED
#define blue PG_TOKEN_BLUE
#define green PG_TOKEN_GREEN

#define UP PG_BUTTON_UP
#define DOWN PG_BUTTON_DOWN
#define LEFT PG_BUTTON_LEFT
#define RIGHT PG_BUTTON_RIGHT
#define CONFIRM PG_BUTTON_CONFIRM
#define OPTION PG_BUTTON_OPTION

#define up PG_BUTTON_UP
#define down PG_BUTTON_DOWN
#define left PG_BUTTON_LEFT
#define right PG_BUTTON_RIGHT
#define confirm PG_BUTTON_CONFIRM
#define option PG_BUTTON_OPTION

int pg_grid_init(PgGrid* grid, int width, int height);
void pg_grid_free(PgGrid* grid);
void pg_grid_clear(PgGrid* grid, int value);

bool pg_grid_in_bounds(const PgGrid* grid, int x, int y);
int pg_grid_index(const PgGrid* grid, int x, int y);

bool pg_grid_set(PgGrid* grid, int x, int y, int value);
int pg_grid_get(PgGrid* grid, int x, int y);

bool pg_grid_set_at(PgGrid* grid, const PgCursor* cursor, int value);
int pg_grid_get_at(PgGrid* grid, const PgCursor* cursor);

void pg_cursor_move(PgCursor* cursor, int dx, int dy);
void pg_cursor_clamp(PgCursor* cursor);

void pg_set_input_reader(PgInputReader reader);
PgInput pg_read_input();
bool pg_input_pressed(PgInput input, int button);

void pg_set_draw_handler(PgDrawHandler handler);
void pg_draw();

void pg_swap_int(int* a, int* b);

void pg_stop();
int pg_run(PgRoutine start, PgRoutine frame);

const char* pg_game_name();

#endif /* POCKETGAME_H */
