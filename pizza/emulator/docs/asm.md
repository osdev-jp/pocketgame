# PocketGameアセンブラ
PocketGameアセンブラは、C言語としてコンパイルできる関数呼び出し形式のゲーム向け命令セットです。

各命令はCマクロとして定義されており、ヘッダーをincludeすればSDLを用いたエミュレーターで起動させれますし、PocketGameアセンブリにもなります

## GAME

```c
GAME(name);
```

ゲーム名を定義します。
1つのプログラム内で複数回定義してはいけません。

## GRID

```c
GRID(name, width, height);
```

2Dグリッドを作成します。

```c
GRID(board, 8, 8);
```

この例では、`board`という名前の8x8グリッドを作成します。

グリッドは、盤面、マップ、タイル配置などに使います。

## CURSOR

```c
CURSOR(name, grid);
```

指定したグリッド上を動くカーソルを作成します。

```c
CURSOR(cursor, board);
```

この例では、`board`上を移動する`cursor`を作成します。

カーソルは、盤面上の選択位置や操作対象の位置を表します。

## SELECT

```c
SELECT(grid, cursor);
```

グリッドにカーソルを関連付けます。

```c
SELECT(board, cursor);
```

この命令を使うと、`DRAW(board);`のときに`board`に対応するカーソルも描画対象になります。

## STATE

```c
STATE(name, value);
```

ゲーム状態を表す変数を作成します。

```c
STATE(turn, black);
STATE(enemy, white);
```

この例では、現在の手番を`turn`、相手側を`enemy`として保存します。

内部的には`int`変数として定義されます。

## ON_START

```c
ON_START(function);
```

ゲーム開始時に一度だけ呼び出す関数を登録します。

```c
ON_START(start);
```

この例では、ゲーム開始時に`start()`が実行されます。

主に初期配置、初期状態設定、画面初期化に使います。

## ON_FRAME

```c
ON_FRAME(function);
```

毎フレーム呼び出す関数を登録します。

```c
ON_FRAME(input);
```

この例では、毎フレーム`input()`が実行されます。

主に入力処理、更新処理に使います。

## SET_CELL

```c
SET_CELL(grid, x, y, value);
```

グリッドの指定位置に値を設定します。

```c
SET_CELL(board, 3, 3, white);
```

この例では、`board` の `(3, 3)` に `white` を設定します。

オセロなら石の配置、RPGならマップタイルの配置、テトリスならブロックの配置に使えます。

## SET_CELL_AT

```c
SET_CELL_AT(grid, cursor, value);
```

カーソル位置に値を設定します。

```c
SET_CELL_AT(board, cursor, turn);
```

この例では、`cursor`が指している`board`のマスに`turn`の値を設定します。

## GET_CELL

```c
GET_CELL(dst, grid, x, y);
```

グリッドの指定位置から値を読み取り、`dst`に保存します。

```c
GET_CELL(cell, board, 3, 3);
```

この例では、`board`の `(3, 3)`の値を`cell`に保存します。

## CELL_AT

```c
CELL_AT(dst, grid, cursor);
```

カーソル位置のセル値を読み取り、`dst`に保存します。

```c
CELL_AT(cell, board, cursor);
```

この例では、`cursor`が指している`board`のマスの値を`cell`に保存します。

## INPUT

```c
INPUT(name);
```

入力状態を保存する変数を作成します。

```c
INPUT(pad);
```

この例では、入力状態を保存する`pad`を作成します。

## READ_INPUT

```c
READ_INPUT(dst);
```

現在の入力状態を読み取り、`dst`に保存します。

```c
READ_INPUT(pad);
```

この例では、現在の入力状態を`pad`に保存します。

## PRESSED

```c
PRESSED(dst, input, button);
```

指定したボタンが押されたかを調べます。

```c
PRESSED(ok, pad, UP);
```

この例では、`pad`の中で`UP`が押されていれば`ok`に`true`、押されていなければ`false`を保存します。

## FLAG

```c
FLAG(name);
```

真偽値を保存するフラグを作成します。

```c
FLAG(ok);
```

この例では、判定結果を保存する `ok`を作成します。

## IF

```c
IF(condition, function);
```

条件が真なら、指定した関数を呼び出します。

```c
IF(ok, cursor_up);
```

この例では、`ok` が `true` のとき `cursor_up()`を呼び出します。

これはジャンプではなく、関数呼び出しです。
呼び出された関数が終わると、元の処理に戻ります。

## MOVE

```c
MOVE(target, dx, dy);
```

対象を指定量だけ移動します。

```c
MOVE(cursor, 0, 1);
```

この例では、`cursor`をY方向に1移動します。

カーソルやエンティティの移動に使います。

## CLAMP_TO_GRID

```c
CLAMP_TO_GRID(target);
```

対象が所属するグリッドの外に出ないように補正します。

```c
CLAMP_TO_GRID(cursor);
```

この例では、`cursor`が`board`の範囲外に出ないようにします。

## SET

```c
SET(dst, value);
```

値を設定します。

```c
SET(score, 0);
```

この例では、`score`に`0`を設定します。

## ADD

```c
ADD(dst, value);
```

値を加算します。

```c
ADD(score, 10);
```

この例では、`score`に`10`を加算します。

## SUB

```c
SUB(dst, value);
```

値を減算します。

```c
SUB(life, 1);
```

この例では、`life`から`1`を減算します。

## SWAP

```c
SWAP(a, b);
```

2つの値を入れ替えます。

```c
SWAP(turn, enemy);
```

この例では`turn`と`enemy`の値を入れ替えます。
ターン制ゲームの手番交代などに使えます。

## GRAPHIC

```c
GRAPHIC();
```

ウィンドウを開きます。
この命令は内部的に `pg_graphic_open(WIDTH, HEIGHT)` を呼び出します。

## DRAW

```c
DRAW(target);
```

指定した対象を描画します。

```c
DRAW(board);
```

この例では、`board`を描画します。

`DRAW(board);`は、画面のクリア、グリッド描画、カーソル描画、画面反映までをまとめて行う想定です。

## RUN

```c
RUN();
```

登録された開始処理とフレーム処理を実行します。
直接書かず、`POCKETGAME_MAIN()`を使います。

## POCKETGAME_MAIN
```c
POCKETGAME_MAIN();
```

1つのプログラムに1回だけ書きます。書いておけばOKです（最後に）

## example
- [オセロ](../examples/osero.c)