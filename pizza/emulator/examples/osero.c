#include <stdbool.h>

#include "pocketgame.h"

GAME(osero);

GRID(board, 8, 8);
CURSOR(cursor, board);

STATE(turn, black);
STATE(enemy, white);
STATE(pass_count, 0);

INPUT(pad);
FLAG(ok);
FLAG(game_over);

MESSAGE(black_win_message, "Black wins!");
MESSAGE(white_win_message, "White wins!");
MESSAGE(draw_message, "Draw!");
MESSAGE(black_pass_message, "Black passes.");
MESSAGE(white_pass_message, "White passes.");

void start(void);
void input(void);
void cursor_up(void);
void cursor_down(void);
void cursor_left(void);
void cursor_right(void);
void try_place(void);
void switch_turn(void);
bool is_in_board(int x, int y);
int count_flips_in_direction(int x, int y, int dx, int dy, int stone);
int count_total_flips(int x, int y, int stone);
void flip_in_direction(int x, int y, int dx, int dy, int stone);
void flip_stones(int x, int y, int stone);
bool has_valid_move(int stone);
void advance_turn(void);
void finish_game(void);
void count_stones(int* black_count, int* white_count);

ON_START(start);
ON_FRAME(input);

void start(void) {
  GRAPHIC();
  SELECT(board, cursor);

  turn = black;
  enemy = white;
  pass_count = 0;
  game_over = false;

  SET_CELL(board, 3, 3, white);
  SET_CELL(board, 4, 4, white);
  SET_CELL(board, 3, 4, black);
  SET_CELL(board, 4, 3, black);

  DRAW(board);
}

void input(void) {
  if (game_over) {
    return;
  }

  READ_INPUT(pad);

  PRESSED(ok, pad, UP);
  if (ok) {
    cursor_up();
  }

  PRESSED(ok, pad, DOWN);
  if (ok) {
    cursor_down();
  }

  PRESSED(ok, pad, LEFT);
  if (ok) {
    cursor_left();
  }

  PRESSED(ok, pad, RIGHT);
  if (ok) {
    cursor_right();
  }

  PRESSED(ok, pad, CONFIRM);
  if (ok) {
    try_place();
  }
}

void cursor_up(void) {
  MOVE(cursor, 0, 1);
  CLAMP_TO_GRID(cursor);
  DRAW(board);
}

void cursor_down(void) {
  MOVE(cursor, 0, -1);
  CLAMP_TO_GRID(cursor);
  DRAW(board);
}

void cursor_left(void) {
  MOVE(cursor, -1, 0);
  CLAMP_TO_GRID(cursor);
  DRAW(board);
}

void cursor_right(void) {
  MOVE(cursor, 1, 0);
  CLAMP_TO_GRID(cursor);
  DRAW(board);
}

void try_place(void) {
  int x;
  int y;
  int cell;
  int flips;

  GET_CURSOR_POSITION(x, y, cursor);
  GET_CELL(cell, board, x, y);

  if (cell != empty) {
    return;
  }

  flips = count_total_flips(x, y, turn);

  if (flips <= 0) {
    return;
  }

  SET_CELL_AT(board, cursor, turn);
  flip_stones(x, y, turn);
  pass_count = 0;
  DRAW(board);
  advance_turn();
}

void switch_turn(void) { SWAP(turn, enemy); }

bool is_in_board(int x, int y) {
  return x >= 0 && x < 8 && y >= 0 && y < 8;
}

int count_flips_in_direction(int x, int y, int dx, int dy, int stone) {
  int cx;
  int cy;
  int cell;
  int count;
  int opponent;

  opponent = stone == black ? white : black;
  cx = x + dx;
  cy = y + dy;
  count = 0;

  while (is_in_board(cx, cy)) {
    GET_CELL(cell, board, cx, cy);

    if (cell == opponent) {
      count += 1;
      cx += dx;
      cy += dy;
      continue;
    }

    if (cell == stone && count > 0) {
      return count;
    }

    return 0;
  }

  return 0;
}

int count_total_flips(int x, int y, int stone) {
  int total;

  total = 0;
  total += count_flips_in_direction(x, y, -1, -1, stone);
  total += count_flips_in_direction(x, y, 0, -1, stone);
  total += count_flips_in_direction(x, y, 1, -1, stone);
  total += count_flips_in_direction(x, y, -1, 0, stone);
  total += count_flips_in_direction(x, y, 1, 0, stone);
  total += count_flips_in_direction(x, y, -1, 1, stone);
  total += count_flips_in_direction(x, y, 0, 1, stone);
  total += count_flips_in_direction(x, y, 1, 1, stone);

  return total;
}

void flip_in_direction(int x, int y, int dx, int dy, int stone) {
  int count;

  count = count_flips_in_direction(x, y, dx, dy, stone);

  for (int i = 1; i <= count; i++) {
    SET_CELL(board, x + dx * i, y + dy * i, stone);
  }
}

void flip_stones(int x, int y, int stone) {
  flip_in_direction(x, y, -1, -1, stone);
  flip_in_direction(x, y, 0, -1, stone);
  flip_in_direction(x, y, 1, -1, stone);
  flip_in_direction(x, y, -1, 0, stone);
  flip_in_direction(x, y, 1, 0, stone);
  flip_in_direction(x, y, -1, 1, stone);
  flip_in_direction(x, y, 0, 1, stone);
  flip_in_direction(x, y, 1, 1, stone);
}

bool has_valid_move(int stone) {
  int x;
  int y;
  int cell;

  for (y = 0; y < 8; y++) {
    for (x = 0; x < 8; x++) {
      GET_CELL(cell, board, x, y);

      if (cell != empty) {
        continue;
      }

      if (count_total_flips(x, y, stone) > 0) {
        return true;
      }
    }
  }

  return false;
}

void advance_turn(void) {
  switch_turn();

  if (has_valid_move(turn)) {
    return;
  }

  pass_count += 1;

  if (turn == black) {
    SHOW_MESSAGE(black_pass_message);
  } else {
    SHOW_MESSAGE(white_pass_message);
  }

  switch_turn();

  if (has_valid_move(turn)) {
    return;
  }

  pass_count += 1;
  finish_game();
}

void finish_game(void) {
  int black_count;
  int white_count;

  count_stones(&black_count, &white_count);
  game_over = true;

  if (black_count > white_count) {
    SHOW_MESSAGE(black_win_message);
    return;
  }

  if (white_count > black_count) {
    SHOW_MESSAGE(white_win_message);
    return;
  }

  SHOW_MESSAGE(draw_message);
}

void count_stones(int* black_count, int* white_count) {
  int x;
  int y;
  int cell;

  *black_count = 0;
  *white_count = 0;

  for (y = 0; y < 8; y++) {
    for (x = 0; x < 8; x++) {
      GET_CELL(cell, board, x, y);

      if (cell == black) {
        *black_count += 1;
      } else if (cell == white) {
        *white_count += 1;
      }
    }
  }
}
