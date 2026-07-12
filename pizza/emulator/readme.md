# PocketGame library
PocketGame)のアセンブラをそのままCとして実行できるライブラリです。

## ビルド

```bash
make
```

## 実行

ソースファイルを直接指定して実行できます。

```bash
./emulator.pl source.c
./emulator.pl source.pas
```

また、次のコマンドでも実行できます。

```bash
make run
```

オセロ（`examples/osero.c`）が起動します。

```bash
make osero
```

```bash
./emulator.pl examples/osero.c
```

五目並べを起動する場合:

```bash
make gomoku
```

```bash
./emulator.pl examples/gomoku.c
```

## 依存ライブラリ
- libsdl2-dev

## memo
アセンブラは特定のゲームだけに使える命令じゃなくて汎用的で似たような他のゲームを作るときにも使えるものを目指す

2026 (c) tas0dev
