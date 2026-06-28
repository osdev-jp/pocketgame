GAME(gomoku);

GRID(board, 15, 15);
CURSOR(cursor, board);

STATE(turn, black);
STATE(enemy, white);
STATE(move_count, 0);

INPUT(pad);

FLAG(ok);
FLAG(game_over);
FLAG(five_found);

STATE(cursor_x, 0);
STATE(cursor_y, 0);
STATE(cell, empty);
STATE(axis_dx, 0);
STATE(axis_dy, 0);
STATE(scan_x, 0);
STATE(scan_y, 0);
STATE(scan_dx, 0);
STATE(scan_dy, 0);
STATE(scan_stone, empty);
STATE(scan_count, 0);
STATE(line_count, 0);

MESSAGE(black_win_message, "Black wins!");
MESSAGE(white_win_message, "White wins!");
MESSAGE(draw_message, "Draw!");

void start();
void input();

void cursor_up();
void cursor_down();
void cursor_left();
void cursor_right();

void try_place();
void switch_turn();

void count_direction();
void check_line();
void has_five();

ON_START(start);
ON_FRAME(input);

void start() {
  GRAPHIC();

  SELECT(board, cursor);

  MOVE(cursor, 7, 7);
  CLAMP_TO_GRID(cursor);

  MOV(move_count, 0);
  MOV(game_over, false);
  MOV(five_found, false);

  DRAW(board);
}

void input() {
  TEST(game_over);
  JZ(input_active);
  JMP(input_end);

  LABEL(input_active);

  READ_INPUT(pad);

  PRESSED(ok, pad, UP);
  TEST(ok);
  JZ(input_skip_up);
  cursor_up();
  LABEL(input_skip_up);

  PRESSED(ok, pad, DOWN);
  TEST(ok);
  JZ(input_skip_down);
  cursor_down();
  LABEL(input_skip_down);

  PRESSED(ok, pad, LEFT);
  TEST(ok);
  JZ(input_skip_left);
  cursor_left();
  LABEL(input_skip_left);

  PRESSED(ok, pad, RIGHT);
  TEST(ok);
  JZ(input_skip_right);
  cursor_right();
  LABEL(input_skip_right);

  PRESSED(ok, pad, CONFIRM);
  TEST(ok);
  JZ(input_skip_confirm);
  try_place();
  LABEL(input_skip_confirm);

  LABEL(input_end);
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
  GET_CURSOR_POSITION(
    cursor_x,
    cursor_y,
    cursor
  );

  GET_CELL(
    cell,
    board,
    cursor_x,
    cursor_y
  );

  CMP(cell, empty);
  JNE(try_place_end);

  SET_CELL_AT(board, cursor, turn);
  ADD(move_count, move_count, 1);

  DRAW(board);

  has_five();

  TEST(five_found);
  JZ(try_place_check_draw);

  MOV(game_over, true);

  CMP(turn, black);
  JNE(try_place_white_win);

  SHOW_MESSAGE(black_win_message);
  JMP(try_place_end);

  LABEL(try_place_white_win);

  SHOW_MESSAGE(white_win_message);
  JMP(try_place_end);

  LABEL(try_place_check_draw);

  CMP(move_count, 225);
  JL(try_place_switch_turn);

  MOV(game_over, true);
  SHOW_MESSAGE(draw_message);

  JMP(try_place_end);

  LABEL(try_place_switch_turn);

  switch_turn();

  LABEL(try_place_end);
}

void count_direction() {
  ADD(scan_x, cursor_x, scan_dx);
  ADD(scan_y, cursor_y, scan_dy);

  MOV(scan_count, 0);

  LABEL(count_direction_loop);
  
  CMP(scan_x, 0);
  JL(count_direction_end);

  CMP(scan_x, 15);
  JGE(count_direction_end);

  CMP(scan_y, 0);
  JL(count_direction_end);
  
  CMP(scan_y, 15);
  JGE(count_direction_end);

  GET_CELL(
    cell,
    board,
    scan_x,
    scan_y
  );

  CMP(cell, scan_stone);
  JNE(count_direction_end);

  ADD(scan_count, scan_count, 1);
  
  ADD(scan_x, scan_x, scan_dx);
  ADD(scan_y, scan_y, scan_dy);

  JMP(count_direction_loop);

  LABEL(count_direction_end);
}

void check_line() {
  MOV(line_count, 1);
  MOV(scan_stone, turn);

  MOV(scan_dx, axis_dx);
  MOV(scan_dy, axis_dy);

  count_direction();

  ADD(
    line_count,
    line_count,
    scan_count
  );

  SUB(scan_dx, 0, axis_dx);
  SUB(scan_dy, 0, axis_dy);

  count_direction();

  ADD(
    line_count,
    line_count,
    scan_count
  );
}

void has_five() {
  MOV(five_found, false);

  MOV(axis_dx, 1);
  MOV(axis_dy, 0);

  check_line();

  CMP(line_count, 5);
  JGE(has_five_found);

  MOV(axis_dx, 0);
  MOV(axis_dy, 1);

  check_line();

  CMP(line_count, 5);
  JGE(has_five_found);

  MOV(axis_dx, 1);
  MOV(axis_dy, 1);

  check_line();

  CMP(line_count, 5);
  JGE(has_five_found);

  MOV(axis_dx, 1);
  MOV(axis_dy, -1);

  check_line();

  CMP(line_count, 5);
  JGE(has_five_found);

  JMP(has_five_end);

  LABEL(has_five_found);

  MOV(five_found, true);

  LABEL(has_five_end);
}

void switch_turn() {
  SWAP(turn, enemy);
}
