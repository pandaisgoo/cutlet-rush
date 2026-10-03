# Windows 建置與執行

## 適用範圍

以下依原 ZIP 的目錄與依賴整理；這次檢查沒有在 Windows 實際執行。第一次請先使用原本可工作的本機環境，不要同時更換編譯器與不確定相容的 Allegro 二進位版本。

原 ZIP 的建置資訊顯示 MinGW-w64 GCC 8.1.0（x86_64、POSIX threads、SEH）；Allegro 標頭顯示 5.2.7。ZIP 中提供的是 `MinGW/bin/make.exe`，不是必須另找的 `mingw32-make.exe`。

新 clone 不含編譯器、SDK、DLL 與字型。需自行準備相容的本機安裝或依適用規範使用既有課程環境。文末提供 Allegro 官方參考，並不保證任意最新版下載與原工具鏈相容。

## 本機目錄

```text
cutlet-rush/
├── MinGW/                              # 本機依賴，Git 忽略
│   └── bin/
│       ├── g++.exe
│       └── make.exe
├── allegro/                            # 本機依賴，Git 忽略
│   ├── include/allegro5/
│   ├── lib/liballegro_monolith.dll.a
│   └── bin/
└── SourceCode/
    ├── Main.cpp
    ├── makefile
    ├── allegro_monolith-5.2.dll          # 執行依賴，Git 忽略
    ├── libgcc_s_seh-1.dll               # 執行依賴，Git 忽略
    ├── libstdc++-6.dll                  # 執行依賴，Git 忽略
    ├── libwinpthread-1.dll              # 執行依賴，Git 忽略
    └── assets/
        ├── font/                       # 需另外準備字型
        ├── image/
        ├── sound/
        ├── gif/
        └── level/
```

Debug 建置另需原 ZIP 中相應的 `allegro_monolith-debug-5.2.dll`。請使用與編譯環境匹配的 DLL；不追蹤依賴不代表執行時不需要依賴。

## PowerShell 指令

先在 `SourceCode`、`MinGW`、`allegro` 的共同父資料夾開啟 PowerShell。確認原本的依賴和素材都已保留，再執行：

```powershell
Set-Location .\SourceCode
$env:Path = "$(Resolve-Path ..\MinGW\bin);$env:Path"

& ..\MinGW\bin\g++.exe --version
if ($LASTEXITCODE -ne 0) { throw 'Compiler check failed.' }

& ..\MinGW\bin\make.exe release
if ($LASTEXITCODE -ne 0) {
    throw 'Build failed. Do not treat an existing game.exe as a newly built executable.'
}

.\game.exe
```

必須確認編譯成功，不能只因舊的 `game.exe` 仍可執行，就認為新原始碼已重新編譯成功。

原 Makefile 第一個 target 是 `debug`，所以這裡明確指定 `release`；需要除錯版時改成 `debug`，並保留對應的執行 DLL。

## 工作目錄與素材

從 `SourceCode` 內啟動遊戲。程式會以 `./assets/image/...`、`./assets/sound/...`、`./assets/font/...` 等相對於工作目錄的路徑載入素材；從 repository 根目錄啟動可能找不到資源。

`highscore.txt` 是本機玩家紀錄。`Player.cpp` 會檢查輸入檔是否成功開啟，並在記錄成績時寫入檔案，因此現有的本機成績檔不納入追蹤。

字型需求詳見 [字型資料夾說明](../SourceCode/assets/font/README.md)。只有原始碼、但缺少字型的 clone 不能當作已具備完整遊玩條件。

## 已知設定事項

Windows Makefile 的 `-L` 參數目前指向 import-library 檔案而非資料夾；連結步驟同時又直接傳入該 import-library 路徑。這次沒有修改 Makefile，也不據此斷言原專案無法編譯。請先確認本機建置結果，再單獨整理建置系統。

`--test` 不是已驗證的自動化測試：測試模式的建構子提早返回會跳過 `scene_buffer` 初始化，解構子仍會讀取該指標。修正及測試前，不要直接把它用作 CI 成功的證據。

`macOS_allegro_install.sh` 包含以 `sudo` 刪除既有 Allegro 安裝的操作，也會下載並建置依賴。它不是 Windows 操作的一部分，這次也未驗證其 macOS 安裝結果。

## 官方參考

- Allegro releases：<https://github.com/liballeg/allegro5/releases>
- Allegro 文件：<https://liballeg.org/a5docs/trunk/>

這兩個來源也出現在原 ZIP 的 README；選擇下載版本時，仍需核對編譯器與架構相容性。
