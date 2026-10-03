# 本專案第一次上傳 GitHub：操作指南

## 這份設定檔包包含什麼

這是文件與 Git 設定檔包，不是完整遊戲，也不是已編譯的執行版本。

新增根目錄 `README.md`、`.gitignore` 和 `docs/`，並更新 `SourceCode/README.md`、`SourceCode/.gitignore`；另外在字型資料夾補上需求說明。沒有修改任何 `.cpp`、`.h`、Makefile 或遊戲素材，也不包含字型檔、SDK 或執行檔。

先完整保留原 ZIP 與原本能運作的專案。下面的整理在另一份工作副本中進行。

## 1. 建立新的本機 repository

安裝並登入 GitHub Desktop。選 `File → New repository`，填入：

```text
Name: cutlet-rush
Description: A C++17 and Allegro 5 game with tower-defense and conveyor-belt modes.
Local path: 選一個你方便管理的父資料夾，例如 Documents\GitHub
Initialize this repository with a README: 勾選
Git ignore: None
License: None
```

按 `Create repository`。GitHub Desktop 會在 Local path 下建立 `cutlet-rush` 資料夾。此時只是本機倉庫，不必先到 GitHub 網站另外建立同名 repository。

## 2. 複製原專案，再合併設定檔包

從原 ZIP 解壓後的真正專案目錄，複製 `SourceCode/`、`MinGW/`、`allegro/` 三個資料夾到新的 `cutlet-rush/`。不必複製根目錄 `.vscode/`。

接著把本設定檔包內的檔案與資料夾合併到 `cutlet-rush/`，不要把設定檔包的外層資料夾再多包一層。若詢問同名檔案，採用設定檔包版本，包含根目錄 README、`SourceCode/README.md`、`SourceCode/.gitignore`。這些只調整文件與 Git 規則，不改遊戲原始碼。

在完成這一步、確認 `.gitignore` 已放到根目錄之前，不要提交專案檔案。

確認不是 `cutlet-rush/CSI2P-Final-Project-Template-ver-final/SourceCode/` 這種多包一層的配置，而是：

```text
cutlet-rush/
├── README.md
├── .gitignore
├── docs/
├── SourceCode/
├── MinGW/       # 本機保留、Git 忽略
└── allegro/     # 本機保留、Git 忽略
```

忽略 `MinGW`、`allegro`、DLL 和字型，不代表刪除本機檔案；它們仍可能是執行遊戲的必要依賴。請勿將原 ZIP 也複製進 repository。

## 3. 檢查待提交檔案

回到 GitHub Desktop 的 `Changes`。

應該看到 README、`.gitignore`、`docs`、`SourceCode` 的原始碼與待確認授權的圖片、音效、關卡資料。第一次先以 Private 保存與整理，不要跳過公開前權利檢查。

不應看到 `MinGW/`、根目錄 `allegro/`、任何 `.vscode/`、`.exe`、`.dll`、`.ttf`、`.log`、`SourceCode/highscore.txt` 或原 ZIP。

根據目前這一份 ZIP，檔案數應在約 180 個的量級，而不是一萬多個。數量是檢查用參考，不是 GitHub 規定。

若仍出現編譯器、DLL、字型或大量工具檔，先不要 commit；檢查 `.gitignore` 是否真的位於 `cutlet-rush/.gitignore`，而不是被存成 `.gitignore.txt` 或放進別的子資料夾。這些步驟假設全新倉庫；已追蹤的檔案不會因新增 ignore 規則自動取消追蹤。

## 4. Commit 與 Publish

在 Summary 輸入：

```text
Add Cutlet Rush source code and project documentation
```

按 `Commit to main`（名稱以目前分支為準）。

接著按 `Publish repository`，第一次保留 `Keep this code private` 勾選，再按 `Publish Repository`。

完成後點 `Repository → View on GitHub`，確認根目錄可看到專案介紹，原始碼可展開閱讀，而且沒有把整個 ZIP 或工具鏈傳上去。

## 5. 公開前還需要處理

README 與 `docs/CREDITS.md` 的 TODO 要依真實情況補上姓名、團隊分工、模板來源和素材來源。保留第三方聲明，不要把整套課程模板當成全部原創，也不要擅自替所有內容套一個 MIT License。

確認課程允許公開、團隊同意、素材及第三方程式可以公開後，才考慮將 repository 改為 Public。這份整理不是授權審查。

照 `docs/BUILD_WINDOWS.md` 在新工作副本重新建置並執行，檢查兩個模式、按鍵、音效、字型與結算。這次只做靜態檢查，沒有驗證 Windows 執行結果。

新增真正的遊戲截圖與展示影片連結。GitHub 原始碼倉庫和「玩家下載即玩」的發行包是不同的；新的 clone 還需要安裝依賴與另外提供字型，不要在 README 宣稱開箱即玩。

## 之後更新

只修改已經加入 GitHub Desktop 的這份工作副本，流程為：儲存修改 → 檢查 Changes → 填 Summary → Commit → Push origin。

## 官方操作文件

- 建立與使用 GitHub Desktop 倉庫：https://docs.github.com/en/desktop/overview/creating-your-first-repository-using-github-desktop
- 發布既有本機專案：https://docs.github.com/en/desktop/adding-and-cloning-repositories/adding-an-existing-project-to-github-using-github-desktop
- 忽略檔案與已追蹤檔案的差異：https://docs.github.com/en/get-started/git-basics/ignoring-files
- GitHub 大檔案限制：https://docs.github.com/en/repositories/working-with-files/managing-large-files/about-large-files-on-github
