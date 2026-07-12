# pcc - PocketGame Compier

pccはPocketGame向けのCソースをC互換のPocketGameアセンブリ（.pas）へコンパイルします。
生成されたアセンブラはGCCでもコンパイルでき、PocketGameライブラリと組み合わせることでLinux上でも実行できます。（まだ作ってません）

## ビルド

```sh
make
```

## 使い方

```sh
./out/pcc -S examples/osero.c -o out/osero.pas
```

標準出力へ出す場合:

```sh
./out/pcc -S examples/osero.c
```

## オプション

```text
-S          PocketGame アセンブリを生成
-o <file>   出力先を指定
-h          ヘルプを表示
--version   バージョンを表示
```

生成物はPocketGameアセンブリであると同時に、有効なCソースでもあります！
