#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED

#include <allegro5/allegro.h>
#include "UI.h"
#include "shapes/Rectangle.h"

/**
 * @brief Main class that runs the whole game.
 * @details All game procedures must be processed through this class.
 */
class Game
{
public:
	void execute();
public:
	Game(bool testMode = false);
	~Game();
	void game_init();
	bool game_update();
	void game_draw();
    // ★★★ 新增：清空戰場與重置遊戲狀態的輔助函式 ★★★
    void delete_all();
private:
    /**
     * 這裡對應你流程圖上的每一個方框
     */
    enum class STATE {
        SPLASH,  
        MENU,         // 主畫面 (開始遊戲 / 說明 / 離開)
        LEVEL_SELECT, // 關卡選擇 (選 Level 1 或 2)
        INSTRUCTION,  // 遊戲說明
        SETTING,      // 設定
        LEVEL,        // 實際遊戲進行中 (打殭屍)
        PAUSE,        // 暫停
        RESULT,       // 結算畫面 (顯示 Win/Lose，決定去下一關或回主選單)
        END           // 真正關閉程式
    };
    
    STATE state;

    enum class MenuButton {
        NONE,       // 沒有停在任何按鈕上
        NTU,        // 停在 NTU 看板
        START,      // 停在 START 文字
        CUTLETRUSH, // 停在 CUTLETRUSH 文字
        EXIT        // 停在 EXIT 文字
    };
    MenuButton hovered_button; // 儲存當前懸停的按鈕狀態


    //判斷是否overlap
    Rectangle ntu_rect;
    Rectangle start_rect;
    Rectangle exit_rect;
    Rectangle cutletrush_rect;

    // ★★★ 設定頁面 (SETTING) 相關變數 ★★★
    Rectangle back_btn_rect;  // 返回主選單按鈕 (MAIN MENU)
    Rectangle vol_track_rect; // 音量條的軌道 (底條)
    Rectangle vol_knob_rect;  // 音量條的滑塊 (會動的那個)
    float BGM_volume;         // 當前音量數值 (0.0 ~ 1.0)
    bool is_dragging_vol;     // 是否正在拖曳音量條
    // 在 class Game 裡新增以下成員變數
    Rectangle graphics_low_rect;
    Rectangle graphics_med_rect;
    Rectangle graphics_high_rect;
    ALLEGRO_BITMAP* scene_buffer;  // 用來存一整個畫面

    enum class SettingButton {
        NONE,       // 沒有停在任何按鈕上
        MENUBOTTON  // 停在menubotton
    };

    SettingButton hovered_settingbutton;

    // 新增這兩個變數來紀錄當前進度
    int current_level;  // 紀錄現在是第幾關 (1 或 2)
    bool is_win;

    // 紀錄是否顯示returnmenu
    bool is_return_menu_active;
    Rectangle return_yes_rect;
    Rectangle return_no_rect;
    Rectangle level_menu_btn_rect;

	ALLEGRO_EVENT event;
	ALLEGRO_BITMAP *game_icon;
	ALLEGRO_BITMAP *background;
    // ★ Typhoon 特效動畫資料
    struct TyphoonEffect {
        float x, y;      // 特效位置
        int frame;       // 播放到第幾張圖（0 或 1）
        int timer;       // 計時器（控制播放速度）
        bool active;     // 是否啟動中
    };

    TyphoonEffect ty_effect;   // 新增一個特效控制物件

    // Typhoon 動畫資料
    struct TyAnim {
        float x;         // x 座標
        float y;         // y 座標
        int frame;       // 0 / 1 (影格)
        int timer;       // 控制影格切換速度
    };

    std::vector<TyAnim> ty_list;    // 一群颱風動畫
    ALLEGRO_BITMAP* ty_frame0;
    ALLEGRO_BITMAP* ty_frame1;

    // Wind 動畫資料
    struct WindAnim {
        float x, y;
        float speed;       // 往右速度
        int reveal_step;   // 0 = 1/3, 1 = 1/2, 2 = full
        int timer;         // 每階段停留多久
        bool remove_me;    // 飛出去後刪除
    };

    std::vector<WindAnim> wind_list;
    ALLEGRO_BITMAP* wind_img = nullptr;

    // ★★★ 新增：生存模式倒數計時器 ★★★
    int level_timer;
private:
	ALLEGRO_DISPLAY *display;
	ALLEGRO_TIMER *timer;
	ALLEGRO_EVENT_QUEUE *event_queue;
	UI *ui;
};

#endif
