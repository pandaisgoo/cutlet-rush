# Cutlet Rush

使用 **C++17** 與 **Allegro 5** 開發的課程遊戲專案，包含格子式塔防與輸送帶進攻兩種模式。

> **驗證說明：**本文件依據原始碼整理；本次文件更新未重新驗證 Windows 建置或實際遊戲執行。第三方程式與素材的授權仍需個別確認。

## 專案介紹

第一關由玩家配置機器人、管理資源，抵禦持續靠近的學生與特殊敵人；第二關則改由玩家從輸送帶選取學生卡片，部署學生突破預先配置的機器人防線。

本專案以課程提供的起始模板為基礎進行延伸開發。模板來源與第三方資源資訊請參閱下方的「模板來源與致謝」及 [來源與素材說明](docs/CREDITS.md)。

## 原始碼中可辨識的功能

- 兩種可選模式：限時塔防生存、輸送帶學生部署。
- 機器人、學生、投射物與特殊敵人的類別架構，包含碰撞處理與狀態效果。
- 資源收集，以及第一關的颱風、風技能。
- 開場、選單、選關、說明、設定、暫停與結算狀態切換。
- 背景音量、畫質選項與本機成績紀錄。

目前選關程式提供 Level 1 與 Level 2；素材資料夾雖有 `LEVEL3.txt`、`LEVEL4.txt`，並不代表已完成四個可選關卡。

## 操作方式

| 操作 | 原始碼中的用途 |
| --- | --- |
| 滑鼠左鍵 | 選單互動、選擇及放置單位、收集可收集的物件 |
| 滑鼠右鍵 | 取消正在選取的單位 |
| `P` | 第一關暫停／恢復 |
| `T` | 第一關使用已取得的颱風技能 |
| `W` | 第一關使用已取得的風技能 |

技能需要有可用次數。請在本機實際遊玩確認操作結果，再將專案標示為已測試的發行版本。

## 建置與執行

原 ZIP 以 **Windows x64** 環境打包。Makefile 使用 **C++17**；內附標頭顯示 **Allegro 5.2.7**，編譯器建置資訊顯示 **MinGW-w64 GCC 8.1.0**。這些是原始檔案所記錄的版本，不代表其他新版組合已通過測試。

編譯器、Allegro SDK、執行所需 DLL 與字型檔不納入 Git。因此，**新 clone 的 repository 不是下載即玩的完整遊戲包**。請先閱讀 [Windows 建置說明](docs/BUILD_WINDOWS.md)，準備相容環境與必要資源。

執行時的工作目錄必須是 `SourceCode`，因為程式以 `./assets/` 等相對路徑載入素材。

## 目錄結構

```text
.
├── README.md
├── .gitignore
├── docs/
│   ├── BUILD_WINDOWS.md
│   ├── CREDITS.md
│   └── UPLOAD_GUIDE_ZH.md
└── SourceCode/
    ├── Main.cpp
    ├── Game.cpp / Game.h
    ├── UI.cpp / UI.h
    ├── Level.cpp / Level.h
    ├── Player.cpp / Player.h
    ├── makefile
    ├── data/       # 共用資料與圖像、音效、字型、GIF、操作管理
    ├── robots/     # 機器人與投射物
    ├── students/   # 學生類型與行為
    ├── special/    # 特殊敵人與技能相關行為
    ├── shapes/     # 幾何形狀與重疊判斷
    ├── algif5/     # GIF 支援程式碼，來源與授權待核對
    └── assets/     # 關卡與經確認可提供的素材；字型檔預設不追蹤
```

本機開發環境另需將 `MinGW/`、`allegro/` 放在 `SourceCode/` 同一層；這些依賴資料夾由 `.gitignore` 排除，不會出現在 GitHub 檔案清單。

## 成果展示

**TODO：**補上實際執行遊戲時的截圖，以及確認可公開的展示影片連結。背景素材本身不應標示成實際遊戲截圖。

## 驗證狀態與已知限制

這次整理沒有完成重新建置、遊戲操作測試或 Linux／macOS 相容性測試。Makefile 雖有 Unix 分支，但不能據此宣稱跨平台運作已驗證；內附 macOS 安裝腳本會修改系統安裝目錄，未列為建議的快速安裝方式。

`--test` 目前不應視為已驗證的測試套件：靜態檢查發現測試模式建構子提早返回時，`scene_buffer` 可能未初始化，解構子卻會檢查它。第二關的同一放置分支也呼叫了兩次學生建立函式，應確認是否為預期行為後再發布版本。

字型與原生依賴需另外準備。課程模板、第三方程式和素材權利仍待確認；這份文件不替整個專案授予新的再散布權利或套用統一授權。

## 模板來源與致謝

本專案以 [Introduction to Programming II Final Project Template](https://github.com/lightbulb12294/CSI2P2-Final-Project-Template) 為起點進行延伸開發。感謝原模板提供遊戲專案的基礎架構。

- 模板倉庫：[`lightbulb12294/CSI2P2-Final-Project-Template`](https://github.com/lightbulb12294/CSI2P2-Final-Project-Template)
- 原模板標示課程：Introduction to Programming II（Class Hwann-Tzong Chen）

模板及其他第三方程式、素材的來源與授權資訊，請參閱 [來源與素材說明](docs/CREDITS.md)。
