#include "../include/pocketgame.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static PgInputReader pg_input_reader = 0;
static PgDrawHandler pg_draw_handler = 0;
static bool pg_should_stop = false;
bool __pg_test_result = false;

static bool pg_grid_ensure(PgGrid* grid) {
  if (grid == 0) {
    return false;
  }

  if (grid->width <= 0 || grid->height <= 0) {
    return false;
  }

  if (grid->cell != 0) {
    return true;
  }

  return pg_grid_init(grid, grid->width, grid->height) == 0;
}

int pg_grid_init(PgGrid* grid, int width, int height) {
  if (grid == 0 || width <= 0 || height <= 0) {
    return -1;
  }

  int count = width * height;
  int* cell = calloc((size_t)count, sizeof(int));

  if (cell == 0) {
    return -1;
  }

  grid->width = width;
  grid->height = height;
  grid->cell = cell;

  return 0;
}

void pg_grid_free(PgGrid* grid) {
  if (grid == 0) {
    return;
  }

  free(grid->cell);
  grid->cell = 0;
  grid->width = 0;
  grid->height = 0;
}

void pg_grid_clear(PgGrid* grid, int value) {
  if (!pg_grid_ensure(grid)) {
    return;
  }

  int count = grid->width * grid->height;

  for (int i = 0; i < count; i++) {
    grid->cell[i] = value;
  }
}

bool pg_grid_in_bounds(const PgGrid* grid, int x, int y) {
  if (grid == 0) {
    return false;
  }

  return x >= 0 && y >= 0 && x < grid->width && y < grid->height;
}

int pg_grid_index(const PgGrid* grid, int x, int y) {
  if (!pg_grid_in_bounds(grid, x, y)) {
    return -1;
  }

  return y * grid->width + x;
}

bool pg_grid_set(PgGrid* grid, int x, int y, int value) {
  if (!pg_grid_ensure(grid)) {
    return false;
  }

  int index = pg_grid_index(grid, x, y);

  if (index < 0) {
    return false;
  }

  grid->cell[index] = value;
  return true;
}

int pg_grid_get(PgGrid* grid, int x, int y) {
  if (!pg_grid_ensure(grid)) {
    return PG_TOKEN_EMPTY;
  }

  int index = pg_grid_index(grid, x, y);

  if (index < 0) {
    return PG_TOKEN_EMPTY;
  }

  return grid->cell[index];
}

bool pg_grid_set_at(PgGrid* grid, const PgCursor* cursor, int value) {
  if (cursor == 0) {
    return false;
  }

  return pg_grid_set(grid, cursor->x, cursor->y, value);
}

int pg_grid_get_at(PgGrid* grid, const PgCursor* cursor) {
  if (cursor == 0) {
    return PG_TOKEN_EMPTY;
  }

  return pg_grid_get(grid, cursor->x, cursor->y);
}

void pg_cursor_move(PgCursor* cursor, int dx, int dy) {
  if (cursor == 0) {
    return;
  }

  cursor->x += dx;
  cursor->y += dy;
}

void pg_cursor_clamp(PgCursor* cursor) {
  if (cursor == 0 || cursor->grid == 0) {
    return;
  }

  PgGrid* grid = cursor->grid;

  if (cursor->x < 0) {
    cursor->x = 0;
  }

  if (cursor->y < 0) {
    cursor->y = 0;
  }

  if (cursor->x >= grid->width) {
    cursor->x = grid->width - 1;
  }

  if (cursor->y >= grid->height) {
    cursor->y = grid->height - 1;
  }
}

void pg_set_input_reader(PgInputReader reader) { pg_input_reader = reader; }

PgInput pg_read_input(void) {
  if (pg_input_reader != 0) {
    return pg_input_reader();
  }

  PgInput input = {PG_BUTTON_NONE};
  return input;
}

bool pg_input_pressed(PgInput input, int button) {
  return input.button == button;
}

void pg_set_draw_handler(PgDrawHandler handler) { pg_draw_handler = handler; }

void pg_draw(void) {
  if (pg_draw_handler != 0) {
    pg_draw_handler();
  }
}

void pg_swap_int(int* a, int* b) {
  if (a == 0 || b == 0) {
    return;
  }

  int tmp = *a;
  *a = *b;
  *b = tmp;
}

void pg_stop(void) { pg_should_stop = true; }

int pg_run(PgRoutine start, PgRoutine frame) {
  pg_should_stop = false;
  printf("%s\n", pg_game_name());

  if (start != 0) {
    start();
  }

  while (!pg_should_stop) {
    if (frame != 0) {
      frame();
    } else {
      break;
    }
  }

  pg_graphic_close();
  return 0;
}

POCKETGAME_MAIN()
