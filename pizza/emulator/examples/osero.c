#include <stdbool.h>

#include "pocketgame.h"

GAME(osero);

GRID(board, 8, 8);
CURSOR(cursor, board);

STATE(turn, black);
STATE(enemy, white);

INPUT(pad);
FLAG(ok);

void start();
void input();
void cursor_up();
void cursor_down();
void cursor_left();
void cursor_right();
void try_place();
void switch_turn();

ON_START(start);
ON_FRAME(input);

void start() {
  GRAPHIC();
  SELECT(board, cursor);

  SET_CELL(board, 3, 3, white);
  SET_CELL(board, 4, 4, white);
  SET_CELL(board, 3, 4, black);
  SET_CELL(board, 4, 3, black);

  DRAW(board);
}

void input() {
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

void cursor_up() {
  MOVE(cursor, 0, 1);
  CLAMP_TO_GRID(cursor);
  DRAW(board);
}

void cursor_down() {
  MOVE(cursor, 0, -1);
  CLAMP_TO_GRID(cursor);
  DRAW(board);
}

void cursor_left() {
  MOVE(cursor, -1, 0);
  CLAMP_TO_GRID(cursor);
  DRAW(board);
}

void cursor_right() {
  MOVE(cursor, 1, 0);
  CLAMP_TO_GRID(cursor);
  DRAW(board);
}

void try_place() {
  SET_CELL_AT(board, cursor, turn);
  switch_turn();
  DRAW(board);
}

void switch_turn() { SWAP(turn, enemy); }
