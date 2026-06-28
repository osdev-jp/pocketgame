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
  TEST(ok);
  JZ(skip_up);
  cursor_up();
  LABEL(skip_up);

  PRESSED(ok, pad, DOWN);
  TEST(ok);
  JZ(skip_down);
  cursor_down();
  LABEL(skip_down);

  PRESSED(ok, pad, LEFT);
  TEST(ok);
  JZ(skip_left);
  cursor_left();
  LABEL(skip_left);

  PRESSED(ok, pad, RIGHT);
  TEST(ok);
  JZ(skip_right);
  cursor_right();
  LABEL(skip_right);

  PRESSED(ok, pad, CONFIRM);
  TEST(ok);
  JZ(skip_confirm);
  try_place();
  LABEL(skip_confirm);
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

void switch_turn() {
  SWAP(turn, enemy);
}
