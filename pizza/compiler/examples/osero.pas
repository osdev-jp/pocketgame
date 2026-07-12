#include <stdbool.h>
#include <pocketgame.h>

GAME(osero);
GRID(board , 8 , 8);
CURSOR(cursor , board);
STATE(turn , black);
STATE(enemy , white);
STATE(pass_count , 0);
INPUT(pad);
FLAG(ok);
FLAG(game_over);
MESSAGE(black_win_message , "Black wins!");
MESSAGE(white_win_message , "White wins!");
MESSAGE(draw_message , "Draw!");
MESSAGE(black_pass_message , "Black passes.");
MESSAGE(white_pass_message , "White passes.");
void start(void);
void input(void);
void cursor_up(void);
void cursor_down(void);
void cursor_left(void);
void cursor_right(void);
void try_place(void);
void switch_turn(void);
bool is_in_board(int x , int y);
int count_flips_in_direction(int x , int y , int dx , int dy , int stone);
int count_total_flips(int x , int y , int stone);
void flip_in_direction(int x , int y , int dx , int dy , int stone);
void flip_stones(int x , int y , int stone);
bool has_valid_move(int stone);
void advance_turn(void);
void finish_game(void);
void count_stones(int * black_count , int * white_count);
ON_START(start);
ON_FRAME(input);
void start(void) {
	GRAPHIC();
	SELECT(board, cursor);
	(turn = black);
	(enemy = white);
	(pass_count = 0);
	(game_over = false);
	SET_CELL(board, 3, 3, white);
	SET_CELL(board, 4, 4, white);
	SET_CELL(board, 3, 4, black);
	SET_CELL(board, 4, 3, black);
	DRAW(board);
}

void input(void) {
	TEST(game_over);
	JZ(pgc_label_0);
	{
		return;
	}
	pgc_label_0:;
	READ_INPUT(pad);
	PRESSED(ok, pad, UP);
	TEST(ok);
	JZ(pgc_label_1);
	{
		cursor_up();
	}
	pgc_label_1:;
	PRESSED(ok, pad, DOWN);
	TEST(ok);
	JZ(pgc_label_2);
	{
		cursor_down();
	}
	pgc_label_2:;
	PRESSED(ok, pad, LEFT);
	TEST(ok);
	JZ(pgc_label_3);
	{
		cursor_left();
	}
	pgc_label_3:;
	PRESSED(ok, pad, RIGHT);
	TEST(ok);
	JZ(pgc_label_4);
	{
		cursor_right();
	}
	pgc_label_4:;
	PRESSED(ok, pad, CONFIRM);
	TEST(ok);
	JZ(pgc_label_5);
	{
		try_place();
	}
	pgc_label_5:;
}

void cursor_up(void) {
	MOVE(cursor, 0, 1);
	CLAMP_TO_GRID(cursor);
	DRAW(board);
}

void cursor_down(void) {
	MOVE(cursor, 0, (-1));
	CLAMP_TO_GRID(cursor);
	DRAW(board);
}

void cursor_left(void) {
	MOVE(cursor, (-1), 0);
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
	TEST((cell != empty));
	JZ(pgc_label_6);
	{
		return;
	}
	pgc_label_6:;
	(flips = count_total_flips(x, y, turn));
	TEST((flips <= 0));
	JZ(pgc_label_7);
	{
		return;
	}
	pgc_label_7:;
	SET_CELL_AT(board, cursor, turn);
	flip_stones(x, y, turn);
	(pass_count = 0);
	DRAW(board);
	advance_turn();
}

void switch_turn(void) {
	SWAP(turn, enemy);
}

bool is_in_board(int x , int y) {
	return ((((x >= 0) && (x < 8)) && (y >= 0)) && (y < 8));
}

int count_flips_in_direction(int x , int y , int dx , int dy , int stone) {
	int cx;
	int cy;
	int cell;
	int count;
	int opponent;
	(opponent = ((stone == black) ? white : black));
	(cx = (x + dx));
	(cy = (y + dy));
	(count = 0);
	pgc_label_8:;
	TEST(is_in_board(cx, cy));
	JZ(pgc_label_9);
	{
		GET_CELL(cell, board, cx, cy);
		TEST((cell == opponent));
		JZ(pgc_label_10);
		{
			(count += 1);
			(cx += dx);
			(cy += dy);
			JMP(pgc_label_8);
		}
		pgc_label_10:;
		TEST(((cell == stone) && (count > 0)));
		JZ(pgc_label_11);
		{
			return count;
		}
		pgc_label_11:;
		return 0;
	}
	JMP(pgc_label_8);
	pgc_label_9:;
	return 0;
}

int count_total_flips(int x , int y , int stone) {
	int total;
	(total = 0);
	(total += count_flips_in_direction(x, y, (-1), (-1), stone));
	(total += count_flips_in_direction(x, y, 0, (-1), stone));
	(total += count_flips_in_direction(x, y, 1, (-1), stone));
	(total += count_flips_in_direction(x, y, (-1), 0, stone));
	(total += count_flips_in_direction(x, y, 1, 0, stone));
	(total += count_flips_in_direction(x, y, (-1), 1, stone));
	(total += count_flips_in_direction(x, y, 0, 1, stone));
	(total += count_flips_in_direction(x, y, 1, 1, stone));
	return total;
}

void flip_in_direction(int x , int y , int dx , int dy , int stone) {
	int count;
	(count = count_flips_in_direction(x, y, dx, dy, stone));
	{
		int i = 1;
		pgc_label_12:;
		TEST((i <= count));
		JZ(pgc_label_14);
		{
			SET_CELL(board, (x + (dx * i)), (y + (dy * i)), stone);
		}
		pgc_label_13:;
		(i++);
		JMP(pgc_label_12);
		pgc_label_14:;
	}
}

void flip_stones(int x , int y , int stone) {
	flip_in_direction(x, y, (-1), (-1), stone);
	flip_in_direction(x, y, 0, (-1), stone);
	flip_in_direction(x, y, 1, (-1), stone);
	flip_in_direction(x, y, (-1), 0, stone);
	flip_in_direction(x, y, 1, 0, stone);
	flip_in_direction(x, y, (-1), 1, stone);
	flip_in_direction(x, y, 0, 1, stone);
	flip_in_direction(x, y, 1, 1, stone);
}

bool has_valid_move(int stone) {
	int x;
	int y;
	int cell;
	{
		(y = 0);
		pgc_label_15:;
		TEST((y < 8));
		JZ(pgc_label_17);
		{
			{
				(x = 0);
				pgc_label_18:;
				TEST((x < 8));
				JZ(pgc_label_20);
				{
					GET_CELL(cell, board, x, y);
					TEST((cell != empty));
					JZ(pgc_label_21);
					{
						JMP(pgc_label_19);
					}
					pgc_label_21:;
					TEST((count_total_flips(x, y, stone) > 0));
					JZ(pgc_label_22);
					{
						return true;
					}
					pgc_label_22:;
				}
				pgc_label_19:;
				(x++);
				JMP(pgc_label_18);
				pgc_label_20:;
			}
		}
		pgc_label_16:;
		(y++);
		JMP(pgc_label_15);
		pgc_label_17:;
	}
	return false;
}

void advance_turn(void) {
	switch_turn();
	TEST(has_valid_move(turn));
	JZ(pgc_label_23);
	{
		return;
	}
	pgc_label_23:;
	(pass_count += 1);
	TEST((turn == black));
	JZ(pgc_label_24);
	{
		SHOW_MESSAGE(black_pass_message);
	}
	JMP(pgc_label_25);
	pgc_label_24:;
	{
		SHOW_MESSAGE(white_pass_message);
	}
	pgc_label_25:;
	switch_turn();
	TEST(has_valid_move(turn));
	JZ(pgc_label_26);
	{
		return;
	}
	pgc_label_26:;
	(pass_count += 1);
	finish_game();
}

void finish_game(void) {
	int black_count;
	int white_count;
	count_stones((&black_count), (&white_count));
	(game_over = true);
	TEST((black_count > white_count));
	JZ(pgc_label_27);
	{
		SHOW_MESSAGE(black_win_message);
		return;
	}
	pgc_label_27:;
	TEST((white_count > black_count));
	JZ(pgc_label_28);
	{
		SHOW_MESSAGE(white_win_message);
		return;
	}
	pgc_label_28:;
	SHOW_MESSAGE(draw_message);
}

void count_stones(int * black_count , int * white_count) {
	int x;
	int y;
	int cell;
	((*black_count) = 0);
	((*white_count) = 0);
	{
		(y = 0);
		pgc_label_29:;
		TEST((y < 8));
		JZ(pgc_label_31);
		{
			{
				(x = 0);
				pgc_label_32:;
				TEST((x < 8));
				JZ(pgc_label_34);
				{
					GET_CELL(cell, board, x, y);
					TEST((cell == black));
					JZ(pgc_label_35);
					{
						((*black_count) += 1);
					}
					JMP(pgc_label_36);
					pgc_label_35:;
					TEST((cell == white));
					JZ(pgc_label_37);
					{
						((*white_count) += 1);
					}
					pgc_label_37:;
					pgc_label_36:;
				}
				pgc_label_33:;
				(x++);
				JMP(pgc_label_32);
				pgc_label_34:;
			}
		}
		pgc_label_30:;
		(y++);
		JMP(pgc_label_29);
		pgc_label_31:;
	}
}

