# PocketGame library
PocketGame)のアセンブラをそのままCとして実行できるライブラリです。

## ビルド
1. このリポジトリをクローンします。
2. `make`の後、`make run`します。

デフォルトではオセロが起動しますが、`make gomoku`で`example/gomoku.c`を起動することができます。
オセロを明示的に起動する際は`make osero`です。

フォーマットは`make fmt`です。

## 依存ライブラリ
- libsdl2-dev

## memo
アセンブラは特定のゲームだけに使える命令じゃなくて汎用的で似たような他のゲームを作るときにも使えるものを目指す

2026 (c) tas0dev
