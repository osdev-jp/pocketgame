# PocketGameアセンブラ

PocketGameアセンブラは、C言語としてコンパイルできる関数呼び出し形式のゲーム向け命令セットです。

各命令はCマクロとして定義されています。`pocketgame.h`をインクルードすると、同じソースコードを次の2つの用途で利用できます。
通常の関数定義と関数呼び出しには、C言語の構文をそのまま使用します。

```c
void update();

void update() {
  move_player();
}
```

条件分岐にはC言語の`if`を使用せず、`TEST`、`CMP`、ジャンプ命令、`LABEL`を使用します。

## GAME

```c
GAME(name);
```

ゲーム名を定義します。

1つのプログラム内で複数回定義してはいけません。
ゲーム名は実行ファイルやゲーム情報に格納されるメタデータです。

## GRID

```c
GRID(name, width, height);
```

2次元グリッドを作成します。

```c
GRID(board, 8, 8);
```

この例では、`board`という名前の8×8グリッドを作成します。

グリッドは盤面、マップ、タイル配置などに使用します。

初期状態では、すべてのセルに`empty`が設定されます。

## CURSOR

```c
CURSOR(name, grid);
```

指定したグリッド上を移動するカーソルを作成します。

```c
CURSOR(cursor, board);
```

この例では、`board`上を移動する`cursor`を作成します。

カーソルは盤面上の選択位置や、操作対象の位置を表します。

## SELECT

```c
SELECT(grid, cursor);
```

グリッドにカーソルを関連付けます。

```c
SELECT(board, cursor);
```

この命令を実行すると、`DRAW(board);`でグリッドと対応するカーソルがまとめて描画されます。

## STATE

```c
STATE(name, value);
```

ゲーム状態を保存する変数を作成します。

```c
STATE(turn, black);
STATE(enemy, white);
STATE(score, 0);
```

内部的には整数または数値型の変数として定義されます。

手番、スコア、座標、カウンターなどの保存に使用します。

## FLAG

```c
FLAG(name);
```

真偽値を保存するフラグを作成します。

```c
FLAG(ok);
FLAG(game_over);
```

内部的には`bool`型として定義され、初期値は`false`です。

入力結果、衝突結果、ゲーム終了状態などの保存に使用します。

## INPUT

```c
INPUT(name);
```

入力状態を保存する領域を作成します。

```c
INPUT(pad);
```

この例では、コントローラーの入力状態を保存する`pad`を作成します。

## ON_START

```c
ON_START(function);
```

ゲーム開始時に一度だけ呼び出す関数を登録します。

```c
void start();

ON_START(start);
```

主に初期配置、初期状態の設定、描画システムの初期化に使用します。

## ON_FRAME

```c
ON_FRAME(function);
```

毎フレーム呼び出す関数を登録します。

```c
void frame();

ON_FRAME(frame);
```

主に入力処理、ゲーム状態の更新、描画処理に使用します。

## 関数

関数の定義、前方宣言、呼び出しにはC言語の構文を使用します。

```c
void move_player();

void frame() {
  move_player();
}

void move_player() {
  MOVE(player, 1, 0);
}
```

関数呼び出しが完了すると、呼び出し元の次の命令へ戻ります。

PASM上で`CALL`や`RET`を直接書く必要はありません。

## SET_CELL

```c
SET_CELL(grid, x, y, value);
```

グリッドの指定位置に値を設定します。

```c
SET_CELL(board, 3, 3, white);
```

この例では、`board`の`(3, 3)`に`white`を設定します。

オセロの石、RPGのマップタイル、落ち物ゲームのブロックなどに使用できます。

## SET_CELL_AT

```c
SET_CELL_AT(grid, cursor, value);
```

カーソルが指している位置に値を設定します。

```c
SET_CELL_AT(board, cursor, turn);
```

この例では、`cursor`が指している`board`のセルに`turn`の値を設定します。

## GET_CELL

```c
GET_CELL(dst, grid, x, y);
```

グリッドの指定位置から値を読み取り、`dst`へ保存します。

```c
STATE(cell, empty);

GET_CELL(cell, board, 3, 3);
```

この例では、`board`の`(3, 3)`の値を`cell`へ保存します。

座標がグリッドの範囲外の場合の動作は、実装側で定義します。通常は`empty`を返すか、範囲外エラーとして扱います。

## CELL_AT

```c
CELL_AT(dst, grid, cursor);
```

カーソル位置のセル値を読み取り、`dst`へ保存します。

```c
STATE(cell, empty);

CELL_AT(cell, board, cursor);
```

この例では、`cursor`が指している`board`のセル値を`cell`へ保存します。

## GET_CURSOR_POSITION

```c
GET_CURSOR_POSITION(x_dst, y_dst, cursor);
```

カーソルの現在座標を取得します。

```c
STATE(cursor_x, 0);
STATE(cursor_y, 0);

GET_CURSOR_POSITION(cursor_x, cursor_y, cursor);
```

この例では、カーソルのX座標を`cursor_x`、Y座標を`cursor_y`へ保存します。

## READ_INPUT

```c
READ_INPUT(dst);
```

現在の入力状態を読み取り、指定した入力領域へ保存します。

```c
READ_INPUT(pad);
```

通常は毎フレームの先頭で実行します。

## PRESSED

```c
PRESSED(dst, input, button);
```

指定したボタンが押された瞬間かどうかを調べます。

```c
PRESSED(ok, pad, UP);
```

`UP`が直前のフレームでは押されておらず、現在のフレームで押されている場合に`ok`へ`true`を保存します。

それ以外の場合は`false`を保存します。

利用可能なボタンです。

```c
UP
DOWN
LEFT
RIGHT
CONFIRM
CANCEL
START
```

## MOVE

```c
MOVE(target, dx, dy);
```

対象を指定量だけ移動します。

```c
MOVE(cursor, 0, 1);
```

この例では、`cursor`をY方向へ1移動します。

カーソルやエンティティの移動に使用します。

## CLAMP_TO_GRID

```c
CLAMP_TO_GRID(target);
```

対象が所属するグリッドの範囲外に出ないように座標を補正します。

```c
CLAMP_TO_GRID(cursor);
```

この例では、`cursor`が`board`の範囲外に出ないようにします。

## SET

```c
SET(dst, value);
```

変数へ値を設定します。

```c
SET(score, 0);
SET(game_over, false);
```

C言語の代入に相当します。

```c
score = 0;
```

## ADD

```c
ADD(dst, value);
```

変数へ値を加算します。

```c
ADD(score, 10);
```

この例では、`score`へ10を加算します。

C言語の複合代入に相当します。

```c
score += 10;
```

## SUB

```c
SUB(dst, value);
```

変数から値を減算します。

```c
SUB(life, 1);
```

この例では、`life`から1を減算します。

C言語の複合代入に相当します。

```c
life -= 1;
```

## SWAP

```c
SWAP(a, b);
```

2つの値を入れ替えます。

```c
SWAP(turn, enemy);
```

この例では、`turn`と`enemy`の値を入れ替えます。

ターン制ゲームの手番交代などに使用します。

## ラベル

```c
label_name:
```

ジャンプ先となるラベルを定義します。
ラベルの有効範囲は、ラベルを定義した関数内です。
別の関数内にあるラベルへジャンプしてはいけません。

## TEST

```c
TEST(value);
```

指定した値がゼロまたは`false`かどうかを調べ、比較フラグを更新します。

```c
TEST(ok);
JZ(skip_move);
```

`ok`が`false`なら、`skip_move`へジャンプします。

元の値は変更されません。

## CMP

```c
CMP(left, right);
```

2つの値を比較し、比較フラグを更新します。

```c
CMP(cell, empty);
JNE(place_end);
```

この例では、`cell`と`empty`が異なる場合に`place_end`へジャンプします。

`CMP`自体は値を変更しません。

## JMP

```c
JMP(label);
```

指定したラベルへ無条件でジャンプします。

```c
JMP(loop_start);
```

ジャンプ先のラベルは同じ関数内に定義する必要があります。

## JZ

```c
JZ(label);
```

直前の`TEST`または`CMP`の結果がゼロまたは等しい場合にジャンプします。

```c
TEST(ok);
JZ(skip_input);
```

```c
CMP(cell, empty);
JZ(cell_is_empty);
```

## JNZ

```c
JNZ(label);
```

直前の`TEST`または`CMP`の結果がゼロでない、または等しくない場合にジャンプします。

```c
TEST(game_over);
JNZ(input_end);
```

## JE

```c
JE(label);
```

直前の`CMP`で、左右の値が等しい場合にジャンプします。

```c
CMP(turn, black);
JE(black_turn);
```

`JZ`と同じ条件ですが、比較結果に対して使用すると意図が分かりやすくなります。

## JNE

```c
JNE(label);
```

直前の`CMP`で、左右の値が異なる場合にジャンプします。

```c
CMP(cell, empty);
JNE(place_end);
```

## JL

```c
JL(label);
```

直前の`CMP`で、左側の値が右側の値より小さい場合にジャンプします。

```c
CMP(x, 0);
JL(outside_grid);
```

## JLE

```c
JLE(label);
```

直前の`CMP`で、左側の値が右側の値以下の場合にジャンプします。

```c
CMP(life, 0);
JLE(game_over);
```

## JG

```c
JG(label);
```

直前の`CMP`で、左側の値が右側の値より大きい場合にジャンプします。

```c
CMP(score, high_score);
JG(new_record);
```

## JGE

```c
JGE(label);
```

直前の`CMP`で、左側の値が右側の値以上の場合にジャンプします。

```c
CMP(move_count, 225);
JGE(draw_game);
```

## 条件分岐の例

C言語で次のように書く処理を考えます。

```c
if (ok) {
  cursor_up();
}
```

PASMでは次のように記述します。

```c
TEST(ok);
JZ(skip_up);

cursor_up();

LABEL(skip_up);
```

`ok`が`false`の場合は`skip_up`へジャンプします。

`ok`が`true`の場合だけ`cursor_up()`が呼び出されます。

## ループの例

C言語で次のように書く処理を考えます。

```c
while (x < 15) {
  ADD(x, 1);
}
```

PASMでは次のように記述します。

```c
LABEL(loop_start);

CMP(x, 15);
JGE(loop_end);

ADD(x, 1);

JMP(loop_start);

LABEL(loop_end);
```

## MESSAGE

```c
MESSAGE(name, text);
```

表示用メッセージを定義します。

```c
MESSAGE(win_message, "You win!");
MESSAGE(draw_message, "Draw!");
```

メッセージはゲームリソースとして保持されます。

## SHOW_MESSAGE

```c
SHOW_MESSAGE(message);
```

定義済みのメッセージを画面へ表示します。

```c
SHOW_MESSAGE(win_message);
```

表示位置や表示方法は、PocketGameランタイム側の標準メッセージ表示に従います。

## GRAPHIC

```c
GRAPHIC();
```

描画システムを初期化し、ゲームウィンドウを開きます。

内部的には次の処理を呼び出します。

```c
pg_graphic_open(WIDTH, HEIGHT);
```

通常は`ON_START`に登録した関数内で一度だけ実行します。

## DRAW

```c
DRAW(target);
```

指定した対象を描画します。

```c
DRAW(board);
```

グリッドがカーソルと関連付けられている場合は、グリッドとカーソルをまとめて描画します。

`DRAW`は次の処理をまとめて行う想定です。

- 画面のクリア
- 対象の描画
- 関連するカーソルの描画
- 画面への反映

描画対象の型に応じて、グリッド、エンティティ、タイルマップなどの描画方法が切り替わります。

## RUN

```c
RUN();
```

`ON_START`と`ON_FRAME`で登録された関数を使用して、ゲームループを開始します。

通常のゲームソースから直接呼び出す必要はありません。SDLエミュレーターやPocketGameランタイムが実行開始時に呼び出します。

## 使用例

```c
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
}

void cursor_up() {
  MOVE(cursor, 0, 1);
  CLAMP_TO_GRID(cursor);
  DRAW(board);
}
```

> !NOTE!
> .pasをgccでコンパイルするとLinux / Windows向けのバイナリにできる
> 逆にpocketgame compilerにするとPocketGame向けのバイナリにする