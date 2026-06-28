#include <stdbool.h>
#include "pocketgame.h"

GAME(gomoku);

GRID(board, 15, 15);
CURSOR(cursor, board);

STATE(turn, black);
STATE(enemy, white);
STATE(move_count, 0);

INPUT(pad);

FLAG(ok);
FLAG(game_over);

MESSAGE(black_win_message, "Black wins!");
MESSAGE(white_win_message, "White wins!");
MESSAGE(draw_message, "Draw!");

void start(void);
void input(void);

void cursor_up(void);
void cursor_down(void);
void cursor_left(void);
void cursor_right(void);

void try_place(void);
void switch_turn(void);

int count_direction(
  int start_x,
  int start_y,
  int dx,
  int dy,
  int stone
);

bool has_five(
  int x,
  int y,
  int stone
);

ON_START(start);
ON_FRAME(input);

void start(void) {
  GRAPHIC();

  SELECT(board, cursor);

  // カーソルを15×15盤面の中央へ移動
  MOVE(cursor, 7, 7);
  CLAMP_TO_GRID(cursor);

  move_count = 0;
  game_over = false;

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

  // カーソルが指している盤面座標を取得
  GET_CURSOR_POSITION(x, y, cursor);

  // その座標にあるセルを取得
  GET_CELL(cell, board, x, y);

  // 空きマス以外には置けない
  if (cell != empty) {
    return;
  }

  SET_CELL_AT(board, cursor, turn);

  move_count += 1;

  DRAW(board);

  // 判定そのものはCコードで行う
  if (has_five(x, y, turn)) {
    game_over = true;

    if (turn == black) {
      SHOW_MESSAGE(black_win_message);
    } else {
      SHOW_MESSAGE(white_win_message);
    }

    return;
  }

  // 15×15なので最大225手
  if (move_count >= 225) {
    game_over = true;
    SHOW_MESSAGE(draw_message);
    return;
  }

  switch_turn();
}

int count_direction(
  int start_x,
  int start_y,
  int dx,
  int dy,
  int stone
) {
  int x;
  int y;
  int cell;
  int count;

  x = start_x + dx;
  y = start_y + dy;
  count = 0;

  while (
    x >= 0 &&
    x < 15 &&
    y >= 0 &&
    y < 15
  ) {
    GET_CELL(cell, board, x, y);

    if (cell != stone) {
      break;
    }

    count += 1;

    x += dx;
    y += dy;
  }

  return count;
}

bool has_five(
  int x,
  int y,
  int stone
) {
  int count;

  // 横方向
  count =
    1 +
    count_direction(x, y, -1, 0, stone) +
    count_direction(x, y, 1, 0, stone);

  if (count >= 5) {
    return true;
  }

  // 縦方向
  count =
    1 +
    count_direction(x, y, 0, -1, stone) +
    count_direction(x, y, 0, 1, stone);

  if (count >= 5) {
    return true;
  }

  // 左下から右上への斜め方向
  count =
    1 +
    count_direction(x, y, -1, -1, stone) +
    count_direction(x, y, 1, 1, stone);

  if (count >= 5) {
    return true;
  }

  // 左上から右下への斜め方向
  count =
    1 +
    count_direction(x, y, -1, 1, stone) +
    count_direction(x, y, 1, -1, stone);

  if (count >= 5) {
    return true;
  }

  return false;
}

void switch_turn(void) {
  SWAP(turn, enemy);
}
