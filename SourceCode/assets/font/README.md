# 本機需要另外準備的字型

`SourceCode/data/FontCenter.cpp` 目前使用下列相對路徑：

```text
./assets/font/Caviar_Dreams_Bold.ttf
./assets/font/courbd.ttf
```

本設定檔包不提供字型檔，根目錄 `.gitignore` 也在再散布權利確認前排除字型檔。原本合法取得的本機字型可以留在這個資料夾供本機使用；有檔案不等於已取得公開散布權利。

新 clone 的專案需另外準備可使用的字型，放到程式需要的路徑，或修改 `FontCenter.cpp` 的路徑設定。執行前請確認字型載入成功。請勿只為了讓倉庫看起來完整，就在未確認授權時公開字型檔。
