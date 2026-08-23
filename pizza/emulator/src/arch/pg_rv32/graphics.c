#include "../../../include/graphics.h"

/// 本体の入力状態を読み取ってPocketGameの入力形式に変換
static PgInput pg_graphic_read_input(void);

/// 2つの整数のうち、小さい方を返す
static int pg_min_int(int a, int b);

/// トークンの種類に対応する描画色を設定
static void pg_graphic_set_token_color(int value);

/// 画面を指定された幅と高さで初期化し、入力処理を登録
int pg_graphic_open(int width, int height);

/// 画面や描画に使用しているリソースを解放
void pg_graphic_close(void);

/// グリッド、トークン、カーソルを画面へ描画
void pg_graphic_draw_grid(PgGrid* grid);

/// 指定されたメッセージを画面へ表示
void pg_graphic_show_message(const char* text);

/*
 * ↑これらを実装したらたぶん動きます！
 */