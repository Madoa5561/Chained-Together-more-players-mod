# Chained Together More Players Mod

Chained Together のホスト人数選択を最大16人に拡張する非公式Modです。実装のベースは [UnchainedTogether](https://github.com/xiaoxiao921/UnchainedTogether) です。

## ダウンロードと導入

1. このリポジトリを `Code` → `Download ZIP` でダウンロードして展開します。
2. ゲームを終了します。
3. `ChainedTogetherMax16` 内の `d3d12.dll` だけを、ゲームの `ChainedTogether\Binaries\Win64` フォルダーへコピーします。
4. Windowsから同名ファイルの置き換えを確認されたら、置き換えます。

標準のインストール先は次のとおりです。

```text
C:\Program Files (x86)\Steam\steamapps\common\ChainedTogether\ChainedTogether\Binaries\Win64\d3d12.dll
```

バックアップDLL、README、ソース、パッチはゲームフォルダーへコピーしません。上書き前に現在の `d3d12.dll` を別の場所へコピーしておくと、元に戻せます。`d3d12.dll.pre-chain-sync` は一つ前のMod版であり、バニラ版DLLではありません。

## 実装

- ホスト画面の人数選択を1〜16人に拡張します。
- 選択した人数をゲーム側の最大人数へ反映します。
- 5人以上のとき、ホスト側でプレイヤー間のチェーン接続を補修する処理を追加しています。

## 検証状況

- ゲーム起動とホスト画面で1〜16人の表示、および16人の選択を確認しています。
- 16人の同時接続、5人以上でのチェーン補修、各参加者へのチェーン同期は実プレイで未確認です。

## ライセンス

Modのローダー部分はUnchainedTogetherを使用し、GPL-3.0に従います。ライセンス全文は [`ChainedTogetherMax16/LICENSE-UnchainedTogether.txt`](ChainedTogetherMax16/LICENSE-UnchainedTogether.txt) を参照してください。既存のリポジトリ直下 `LICENSE`（MIT）は保持していますが、上流由来部分のGPL-3.0条件に代わるものではありません。
