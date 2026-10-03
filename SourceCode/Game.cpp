#include "Game.h"
#include "Utils.h"
#include "data/DataCenter.h"
#include "data/OperationCenter.h"
#include "data/SoundCenter.h"
#include "data/ImageCenter.h"
#include "data/FontCenter.h"
#include "Player.h"
#include "Level.h"
#include "Star.h"

#include "students/Student.h"
#include "special/Special.h"
#include "special/SpecialTyphoon.h"
#include "special/SpecialWind.h"
#include "robots/Robot.h"
#include "robots/Beam.h"

#include "robots/RobotGenerator.h"
#include "robots/RobotShield.h"
#include "robots/RobotLight.h"
#include "robots/RobotBomb.h"
#include "robots/RobotIce.h"

#include "shapes/Rectangle.h" 
#include "shapes/Point.h"     // 必須引入 Point，因為我們要建立滑鼠點

#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_acodec.h>
#include <vector>
#include <cstring>
#include <algorithm>

// 這些是「左上角座標」和「寬高」，等一下在 game_init 會轉換
const int NTU_X = 80,  NTU_Y = 560,  NTU_W = 300, NTU_H = 150;
const int START_X = 800, START_Y = 160, START_W = 500, START_H = 140;
const int CUTLETRUSH_X = 95, CUTLETRUSH_Y = 200, CUTLETRUSH_W = 500, CUTLETRUSH_H = 285; 
const int EXIT_X = 980, EXIT_Y = 640, EXIT_W = 250, EXIT_H = 140;

// fixed settings
constexpr char game_icon_img_path[] = "./assets/image/game_icon.png";
constexpr char game_start_sound_path[] = "./assets/sound/growl.wav";
constexpr char background_img_path[] = "./assets/image/StartBackground.jpg";
constexpr char background_sound_path[] = "./assets/sound/BackgroundMusic.ogg";

constexpr char level_sound_path[] = "./assets/sound/level.mp3";           // 關卡音樂
constexpr char menu_sound_path[]  = "./assets/sound/menu.mp3";            // 主畫面音樂
constexpr char win_sound_path[]  = "./assets/sound/win.mp3";            // 獲勝音樂
constexpr char lose_sound_path[]  = "./assets/sound/lose.mp3";            // 失敗音樂

// 背景圖
constexpr char menu_img_path[] = "./assets/image/MenuBackground.png";
constexpr char menu_start_glow_path[] = "./assets/image/cover_start.png"; 
constexpr char menu_exit_glow_path[] = "./assets/image/cover_exit.png"; 
constexpr char menu_ntu_glow_path[] = "./assets/image/cover_ntu.png";
constexpr char menu_cutletrush_glow_path[] = "./assets/image/cover_cutlet.png";
constexpr char win_img_path[] = "./assets/image/Win.png";
constexpr char lose_img_path[] = "./assets/image/Lose.png";
constexpr char level_background_img_path[] = "./assets/image/game.png";
constexpr char level_background_img_Newpath[] = "./assets/image/game_dusktonight.png";
constexpr char setting_img_path[] = "./assets/image/setting.png";
constexpr char setting_glow_img_path[] = "./assets/image/setting_glow.png";
constexpr char news_img_path[] = "./assets/image/news.png";
constexpr char return_menu_img_path[] = "./assets/image/returntomenu.png";
constexpr char level_select_img_path[] = "./assets/image/level_select.jpg";

// 音量條 (放在畫面中間偏右)
const int VOL_TRACK_X = 425; 
const int VOL_TRACK_Y = 300; 
const int VOL_TRACK_W = 175; 
const int VOL_TRACK_H = 20;

// 返回按鈕 (放在畫面下方中間)
const int BACK_BTN_W = 150;
const int BACK_BTN_H = 60;
const int BACK_BTN_Y = 520;

// 格子開始的位置
const int GRID_START_X = 170;//180
const int GRID_START_Y = 140;
const int GRID_W = 60;
const int GRID_H = 80;


constexpr char menu_button_img_path[] = "./assets/image/MenuButton.png";

enum class SettingButton {
    NONE,
    MENUBOTTON,
    G_LOW,
    G_MED,
    G_HIGH
};


/**
 * @brief Game entry.
 * @details The function processes all allegro events and update the event state to a generic data storage (i.e. DataCenter).
 * For timer event, the game_update and game_draw function will be called if and only if the current is timer.
 */
void
Game::execute() {
	DataCenter *DC = DataCenter::get_instance();
	// main game loop
	bool run = true;
	while(run) {
		// process all events here
		al_wait_for_event(event_queue, &event);
		switch(event.type) {
			case ALLEGRO_EVENT_TIMER: {
				run &= game_update();
				game_draw();
				break;
			} case ALLEGRO_EVENT_DISPLAY_CLOSE: { // stop game
				run = false;
				break;
			} case ALLEGRO_EVENT_KEY_DOWN: {
				DC->key_state[event.keyboard.keycode] = true;
				break;
			} case ALLEGRO_EVENT_KEY_UP: {
				DC->key_state[event.keyboard.keycode] = false;
				break;
			} case ALLEGRO_EVENT_MOUSE_AXES: {
				DC->mouse.x = event.mouse.x;
				DC->mouse.y = event.mouse.y;
				break;
			} case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN: {
				DC->mouse_state[event.mouse.button] = true;
				break;
			} case ALLEGRO_EVENT_MOUSE_BUTTON_UP: {
				DC->mouse_state[event.mouse.button] = false;
				break;
			} default: break;
		}
	}
}

/**
 * @brief Initialize all allegro addons and the game body.
 * @details Only one timer is created since a game and all its data should be processed synchronously.
 */
Game::Game(bool testMode) {
	DataCenter *DC = DataCenter::get_instance();
	GAME_ASSERT(al_init(), "failed to initialize allegro.");

	// initialize allegro addons
	bool addon_init = true;
	addon_init &= al_init_primitives_addon();
	addon_init &= al_init_font_addon();
	addon_init &= al_init_ttf_addon();
	addon_init &= al_init_image_addon();
	addon_init &= al_init_acodec_addon();
	GAME_ASSERT(addon_init, "failed to initialize allegro addons.");

	if(testMode) {
		timer = nullptr;
		event_queue = nullptr;
		display = nullptr;
		debug_log("Game initialized in test mode.\n");
		return;
	}

	// initialize events
	bool event_init = true;
	event_init &= al_install_keyboard();
	event_init &= al_install_mouse();
	event_init &= al_install_audio();
	GAME_ASSERT(event_init, "failed to initialize allegro events.");

	// initialize game body
	GAME_ASSERT(
		timer = al_create_timer(1.0 / DC->FPS),
		"failed to create timer.");
	GAME_ASSERT(
		event_queue = al_create_event_queue(),
		"failed to create event queue.");
	GAME_ASSERT(
		display = al_create_display(DC->window_width, DC->window_height),
		"failed to create display.");

	debug_log("Game initialized.\n");
    scene_buffer = nullptr;   // ★ 初始化
	game_init();
}

/**
 * @brief Initialize all auxiliary resources.
 */
void
Game::game_init() {
	DataCenter *DC = DataCenter::get_instance();
	SoundCenter *SC = SoundCenter::get_instance();
	ImageCenter *IC = ImageCenter::get_instance();
	FontCenter *FC = FontCenter::get_instance();
	// set window icon
	game_icon = IC->get(game_icon_img_path);
	al_set_display_icon(display, game_icon);

	// register events to event_queue
    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_keyboard_event_source());
    al_register_event_source(event_queue, al_get_mouse_event_source());
    al_register_event_source(event_queue, al_get_timer_event_source(timer));

	// Rectangle 的建構子是 (x1, y1, x2, y2)，所以要把寬高加上去
    ntu_rect = Rectangle(NTU_X, NTU_Y, NTU_X + NTU_W, NTU_Y + NTU_H);
    start_rect = Rectangle(START_X, START_Y, START_X + START_W, START_Y + START_H);
    exit_rect = Rectangle(EXIT_X, EXIT_Y, EXIT_X + EXIT_W, EXIT_Y + EXIT_H);
    cutletrush_rect = Rectangle(CUTLETRUSH_X, CUTLETRUSH_Y, CUTLETRUSH_X + CUTLETRUSH_W, CUTLETRUSH_Y + CUTLETRUSH_H);

    hovered_button = MenuButton::NONE;

    // ★★★ 初始化 SETTING 頁面元件 ★★★
    // 1. 初始化音量條區域
    vol_track_rect = Rectangle(VOL_TRACK_X, VOL_TRACK_Y, VOL_TRACK_X + VOL_TRACK_W, VOL_TRACK_Y + VOL_TRACK_H);

    // 2. 初始化音量滑塊 (Knob)
    BGM_volume = 1.0f;
    is_dragging_vol = false;
    int knob_w = 25; // 滑塊大小
    int knob_x = VOL_TRACK_X + (VOL_TRACK_W * BGM_volume) - (knob_w / 2);
    int knob_y = VOL_TRACK_Y + (VOL_TRACK_H / 2) - (knob_w / 2);
    vol_knob_rect = Rectangle(knob_x, knob_y, knob_x + knob_w, knob_y + knob_w);

    // ★新增三種畫質設定按鈕
    int gbtn_w = 180, gbtn_h = 60;
    graphics_low_rect    = Rectangle(150, 230, 150 + gbtn_w, 230 + gbtn_h);
    graphics_med_rect    = Rectangle(150, 310, 150 + gbtn_w, 310 + gbtn_h);
    graphics_high_rect   = Rectangle(150, 390, 150 + gbtn_w, 390 + gbtn_h);


    // 3. 初始化返回按鈕 (對應圖片下方的 MAIN MENU)
    // 我們讓它水平置中
    int back_x = (DC->window_width - BACK_BTN_W) / 2;
    back_btn_rect = Rectangle(back_x, BACK_BTN_Y, back_x + BACK_BTN_W, BACK_BTN_Y + BACK_BTN_H);
    
    hovered_settingbutton = SettingButton::NONE;

    is_return_menu_active = false; // 預設不顯示確認視窗


    // 1. Level 右下角的 Menu 按鈕
    // 假設按鈕大小為 80x80，位置在右下角
    int level_btn_w = 80;
    int level_btn_h = 80;
    int level_btn_x = DC->window_width - level_btn_w - 20; // 右邊留 20px 邊距
    int level_btn_y = DC->window_height - level_btn_h - 20; // 下邊留 20px 邊距
    
    level_menu_btn_rect = Rectangle(level_btn_x, level_btn_y, level_btn_x + level_btn_w, level_btn_y + level_btn_h);

    // 2. 確認視窗 (Return to Menu) 的 YES / NO 按鈕
    // 我們需要根據 returntomenu.jpg 的圖片佈局來設定
    // 假設確認視窗是 600x400，置中顯示
    int box_w = 600; 
    int box_h = 400;
    int box_x = (DC->window_width - box_w) / 2; // 視窗置中 X
    int box_y = (DC->window_height - box_h) / 2; // 視窗置中 Y

    // 根據圖片 returntomenu.jpg:
    // YES 按鈕大約在左下方
    int yes_x = box_x + 130;  // 相對於視窗左邊界
    int yes_y = box_y + 210; // 相對於視窗上邊界
    int btn_w = 150; // 按鈕寬度
    int btn_h = 60;  // 按鈕高度
    return_yes_rect = Rectangle(yes_x, yes_y, yes_x + btn_w, yes_y + btn_h);

    // NO 按鈕大約在右下方
    int no_x = yes_x + 190; 
    int no_y = yes_y;
    return_no_rect = Rectangle(no_x, no_y, no_x + btn_w, no_y + btn_h);

    // ★★★ 設定倒數 30 秒 ★★★
    DC = DataCenter::get_instance();
    level_timer = 30 * DC->FPS; // 30秒 * 60FPS = 1800

	// init sound setting
	SC->init();

	// init font setting
	FC->init();

	ui = new UI();
	ui->init();

	DC->level->init();

	DC->player->init();


    //typhoon effect 初設
    ty_effect.active = false;
    ty_effect.frame = 0;
    ty_effect.timer = 0;
    ty_effect.x = 60;   // T.png 的中心點 X（你可以調整）
    ty_effect.y = 300;  // T.png 的中心點 Y（你可以調整）

    // ★★★ 載入颱風動畫影格（你現在完全沒載入！） ★★★
    ty_frame0 = IC->get("./assets/image/special/Typhoon/typhoon_0.png");
    ty_frame1 = IC->get("./assets/image/special/Typhoon/typhoon_1.png");
    wind_img = IC->get("./assets/image/special/Wind/wind.png");
    

	// game start
	background = IC->get(background_img_path);
	debug_log("Game state: change to START\n");
	state = STATE::SPLASH;
	current_level = 1;
	al_start_timer(timer);
}

/**
 * @brief The function processes all data update.
 * @details The behavior of the whole game body is determined by its state.
 * @return Whether the game should keep running (true) or reaches the termination criteria (false).
 * @see Game::STATE
 */
bool Game::game_update() {
    DataCenter *DC = DataCenter::get_instance();
    OperationCenter *OC = OperationCenter::get_instance();
    SoundCenter *SC = SoundCenter::get_instance();
    static ALLEGRO_SAMPLE_INSTANCE *bgm_instance = nullptr;

    static ALLEGRO_SAMPLE_INSTANCE *level_bgm = nullptr; // 控制 level.wav
    static ALLEGRO_SAMPLE_INSTANCE *menu_bgm = nullptr;
    static ALLEGRO_SAMPLE_INSTANCE *win_bgm = nullptr;
    static ALLEGRO_SAMPLE_INSTANCE *lose_bgm = nullptr;

    // 取得滑鼠座標與點擊狀態
    int mouse_x = DC->mouse.x;
    int mouse_y = DC->mouse.y;
    bool is_clicked = DC->mouse_state[1] && !DC->prev_mouse_state[1]; // 左鍵點擊瞬間

    switch(state) {
        // 1. 進場畫面 (Logo)
        case STATE::SPLASH: {
            static int timer = 0;
            timer++;
            if(timer > 0) { // 0表示過場動畫時間(考慮可以調大)
                state = STATE::MENU;
                timer = 0;
            }
            break;
        }

        // 2. 主畫面 (Main Menu)
        case STATE::MENU: {

            
            if(DC->player) 
            {
                DC->player->init(); 
            }
            //放音樂的程式碼
            
            // (A) 如果正在播放關卡音樂 (從遊戲退出來)，先停掉
            if (level_bgm) {
                SC->stop(level_bgm);
                level_bgm = nullptr;
            }

            // (B) 如果選單音樂還沒播，就開始播
            if (!menu_bgm) {
                menu_bgm = SC->play(menu_sound_path, ALLEGRO_PLAYMODE_LOOP);
            }
            

            // 1. 座標轉換 (視窗座標 -> 圖片原始座標)
            ALLEGRO_BITMAP *orig_bg = ImageCenter::get_instance()->get(menu_img_path);
            int bg_w = al_get_bitmap_width(orig_bg);
            int bg_h = al_get_bitmap_height(orig_bg);

            int img_mouse_x = mouse_x * ((float)bg_w / DC->window_width);
            int img_mouse_y = mouse_y * ((float)bg_h / DC->window_height);

            // ★ 4. 建立一個 Point 物件代表滑鼠位置
            Point mouse_point(img_mouse_x, img_mouse_y);

            // ★ 5. 使用 Rectangle::overlap 來偵測碰撞
            if (start_rect.overlap(mouse_point)) {
                hovered_button = MenuButton::START;
            } else if (exit_rect.overlap(mouse_point)) {
                hovered_button = MenuButton::EXIT;
            } else if (ntu_rect.overlap(mouse_point)) {
                hovered_button = MenuButton::NTU;
            } else if (cutletrush_rect.overlap(mouse_point)) {
                hovered_button = MenuButton::CUTLETRUSH;
            } else {
                hovered_button = MenuButton::NONE;
            }

            // 6. 處理點擊 (這部分邏輯跟之前一樣，不用變)
            if(is_clicked) {
                switch(hovered_button) {
                    case MenuButton::START:
                        state = STATE::LEVEL_SELECT;
                        break;
                    case MenuButton::EXIT:
                        return false;
                    case MenuButton::NTU:
                        state = STATE::INSTRUCTION;
                        break;
                    case MenuButton::CUTLETRUSH:
                        state = STATE::SETTING;
                        break;
                    default: break;
                }
            }
            break;
        }

        // 設定
        case STATE::SETTING: {
            // 這裡要用視窗座標 (因為我們是用 al_draw_scaled_bitmap 把背景填滿視窗的)
            Point mouse_p(mouse_x, mouse_y);
            bool is_mouse_down = DC->mouse_state[1];

            // 按下瞬間偵測
            if (is_clicked) {
                if (vol_knob_rect.overlap(mouse_p) || vol_track_rect.overlap(mouse_p)) {
                    is_dragging_vol = true;
                }
            }

            // 放開滑鼠停止拖曳
            if (!is_mouse_down) {
                is_dragging_vol = false;
            }

            // 拖曳中更新數值
            if (is_dragging_vol) {
                float new_vol = (float)(mouse_x - VOL_TRACK_X) / VOL_TRACK_W;
                
                // 限制範圍 0.0 ~ 1.0
                if (new_vol > 1.0f) new_vol = 1.0f;
                if (new_vol < 0.0f) new_vol = 0.0f;
                
                BGM_volume = new_vol;
                
                // 調整全域音量
                al_set_mixer_gain(al_get_default_mixer(), BGM_volume);
            }

            // 更新滑塊圖形位置 (讓它跟著數值跑)
            int knob_w = 25; // 要跟 init 裡設定的一樣
            int current_knob_x = VOL_TRACK_X + (VOL_TRACK_W * BGM_volume) - (knob_w / 2);
            vol_knob_rect.x1 = current_knob_x;
            vol_knob_rect.x2 = current_knob_x + knob_w;

            // 畫質更新
            if (is_clicked) {
                if (graphics_low_rect.overlap(mouse_p)) {
                    DC->graphics_quality = GraphicsQuality::LOW;
                }
                else if (graphics_med_rect.overlap(mouse_p)) {
                    DC->graphics_quality = GraphicsQuality::MEDIUM;
                }
                else if (graphics_high_rect.overlap(mouse_p)) {
                    DC->graphics_quality = GraphicsQuality::HIGH;
                }
            }


            
            if (back_btn_rect.overlap(mouse_p)) {
                hovered_settingbutton = SettingButton::MENUBOTTON;
            } else {
                hovered_settingbutton = SettingButton::NONE;
            }
            // (A) 處理返回按鈕 (MAIN MENU)
            if(is_clicked && back_btn_rect.overlap(mouse_p)) {
                state = STATE::MENU;
                is_dragging_vol = false; // 離開時停止拖曳
            }
            break;
        }

        // 3. 關卡選擇
        case STATE::LEVEL_SELECT: {
            if(is_clicked) {
                // [暫時邏輯] 點左邊 -> Level 1
                if(mouse_x < DC->window_width / 2) {
                    current_level = 1;
                    DC->level->load_level(1);
                    state = STATE::LEVEL;
                }
                // [暫時邏輯] 點右邊 -> Level 2
                else {
                    current_level = 2;
                    DC->player->HP = 1;
                    DC->level->load_level(2);
                    state = STATE::LEVEL;
                }
            }
            DC->levels = current_level;
            break;
        }

        // 4. 遊戲說明
        case STATE::INSTRUCTION: {
            if(is_clicked) {
                state = STATE::MENU; // 點一下回主選單
            }
            break;
        }

        // 5. 實際遊戲 (Level) - 這裡保留原本大部份邏輯
        case STATE::LEVEL: {
            Point mouse_p(mouse_x, mouse_y);
            // ★★★ 情況 A: 正在顯示確認視窗 (暫停遊戲，只處理 YES/NO) ★★★
            if (is_return_menu_active) {
                if (is_clicked) {
                    // 點擊 YES -> 回主選單 (記得清空戰場！)
                    if (return_yes_rect.overlap(mouse_p)) {
                        state = STATE::MENU;
                        // 清除場上的東西
			            delete_all();
                        is_return_menu_active = false; // 重置狀態
                    }
                    // 點擊 NO -> 關閉視窗，繼續遊戲
                    else if (return_no_rect.overlap(mouse_p)) {
                        is_return_menu_active = false;
                    }
                }
                // ★ 重要：直接跳出，不執行下面的遊戲更新，達到暫停效果
                break; 
            }

            // ★★★ 情況 B: 正常遊戲進行中 ★★★

            if (is_clicked && level_menu_btn_rect.overlap(mouse_p)) {
                is_return_menu_active = true;
            }
            // (A) 如果正在播選單音樂，先停掉
            if (menu_bgm) {
                SC->stop(menu_bgm);
                menu_bgm = nullptr;
            }

            // (B) 如果關卡音樂還沒播，就開始播
            if(!level_bgm) {
                level_bgm = SC->play(level_sound_path, ALLEGRO_PLAYMODE_LOOP);
            }

            if(current_level == 1){
                // 按 T 且 typhoon_count >= 1 才觸發颱風群動畫
                if (DC->key_state[ALLEGRO_KEY_T] && !DC->prev_key_state[ALLEGRO_KEY_T]) {

                    if (DC->player->typhoon_count >= 1) {
                        DC->player->typhoon_count--;

                        // 產生颱風動畫
                        ty_list.clear();     // 每次按 T 重新生成一組

                        for (int i = 0; i < 4; i++) {   // 你要的 4 個颱風
                            TyAnim t;
                            t.x = -150 - i * 150;      // 從畫面左外跳出
                            t.y = 150 + i * 80;        // 一高一低交錯
                            t.frame = i % 2;
                            t.timer = 10;
                            ty_list.push_back(t);
                        }

                        debug_log("[Typhoon] T pressed → spawn 4 tornadoes.\n");
                    }
                }

                for (auto &t : ty_list) {

                    if (t.timer > 0) t.timer--;
                    else {
                        t.frame = 1 - t.frame;
                        t.timer = 10;
                    }

                    t.x += 5;

                    // 如果飛出右邊螢幕 → 標記為刪除
                    if (t.x > DC->window_width + 200) {
                        t.x = 999999; // 或任意標記值等待刪除
                    }
                }
                // 清理飛出畫面的颱風
                ty_list.erase(
                    std::remove_if(ty_list.begin(), ty_list.end(),
                        [&](const TyAnim& t) {
                            return t.x > DC->window_width + 200;
                        }),
                    ty_list.end()
                );


                // //---------------------------------------------------------------
                // // ★★★ W 鍵技能：Wind 技能 → 把所有學生往右吹 50px ★★★
                // //---------------------------------------------------------------
                // if (DC->key_state[ALLEGRO_KEY_W] && !DC->prev_key_state[ALLEGRO_KEY_W]) {

                //     // 只有 wind_count >= 1 才能使用技能
                //     if (DC->player->wind_count >= 1) {

                //         DC->player->wind_count--;

                //         debug_log("[WIND] W pressed → blow all students!\n");

                //         for (Student* stu : DC->students) {

                //             // 1. 從 unique_ptr 取出 Shape*（可能是 Rectangle）
                //             Shape* shape = stu->shape.get();
                //             if (!shape) continue;

                //             // 2. 先取得現在的中心點
                //             double cx = shape->center_x();
                //             double cy = shape->center_y();

                //             // 3. 往右吹 50px（你可以改成 80 或 100）
                //             double new_cx = cx + 200;

                //             // 4. 邊界保護：避免吹出螢幕右邊
                //             double max_cx = DC->window_width - 50;  // 安全邊界，可微調
                //             if (new_cx > max_cx)
                //                 new_cx = max_cx;

                //             // 5. 用「更新中心點」的方式平移整個 hitbox
                //             shape->update_center_x(new_cx);

                //             // 2. 更新學生內部的 x（避免下一幀 Student::update() 把他拉回去）
                //             stu->x = new_cx;   // ←←← 必須加這行！
                //         }
                //     }
                //     else {
                //         debug_log("[WIND] W pressed but wind_count == 0 → cannot use.\n");
                //     }
                // }

                //---------------------------------------------------------------
                // ★★★ W 鍵技能：Wind → 吹動所有學生 50px（永久生效）★★★
                //---------------------------------------------------------------
                if (DC->key_state[ALLEGRO_KEY_W] && !DC->prev_key_state[ALLEGRO_KEY_W]) {

                    if (DC->player->wind_count >= 1) {

                        DC->player->wind_count--;

                        debug_log("[WIND] W pressed → blow all students backward!\n");

                        WindAnim w;
                        w.x = -200;
                        w.y = 250;       // 你可以調整高度
                        w.speed = 12;    // 往右速度
                        w.reveal_step = 0;
                        w.timer = 15;    // 每一階段顯示 15 frame
                        w.remove_me = false;

                        wind_list.push_back(w);

                        for (Student* stu : DC->students) {
                            stu->setWindBlown(true);
                            stu->setWindTimer(50);

                            stu->setOldV(stu->getV());  // 記住原本速度
                            stu->setV(-300);             // 向右吹走

                            // // 啟動 WIND 效果
                            // stu->wind_blown = true;

                            // stu->old_v = stu->get_v();  // 記住原本速度（避免 0 或奇怪值）
                            // stu-> = +50;         // ★★★ WIND 技能：往右吹（變成正速度）

                            // stu->wind_timer = 50; // 持續 50 frame
                        }
                    }
                    else {
                        debug_log("[WIND] Not enough wind_count.\n");
                    }
                }

                for (auto &w : wind_list) {

                // (1) reveal 演進（1/3 → 1/2 → full）
                if (w.timer > 0) {
                    w.timer--;
                } else {
                    w.reveal_step++;
                    w.timer = 15;

                    if (w.reveal_step > 2)
                        w.reveal_step = 2;  // 最多 full
                }

                // (2) 移動
                w.x += w.speed;

                // (3) 飛出螢幕 → 標記刪除
                if (w.x > DC->window_width + 300) {
                    w.remove_me = true;
                }
            }

            // 清除飛出螢幕的風
            wind_list.erase(
                std::remove_if(wind_list.begin(), wind_list.end(),
                    [&](const WindAnim& w){ return w.remove_me; }),
                wind_list.end()
            );

                







                if (is_clicked) {
                    for (Star* st : DC->stars)
                    {
                        if (st->collected) continue;

                        if (st->shape.overlap(mouse_p))
                        {
                            st->collected = true;
                            // ★ 設計：讓星星飛向左上角
                            // 你已有 beam->collect() 的飛行寫法，可以共用或複製
                            DC->player->coin+=5; // 或 st->value
                            break;
                        }
                    }
                }

                if (is_clicked) {
                    // 遍歷所有子彈/光束/星星
                    for (Beam* beam : DC->beams) {
                        
                        // 如果已經被點過了正在飛，就不要再點它
                        if (beam->get_is_collected()) continue; 

                        if (beam->shape->overlap(mouse_p)) {
                                
                            //DC->player->coin += 5;
                            // 改成：觸發收集動畫 (錢會在 Beam.cpp 飛到後才加)
                            beam->collect(); 
                                
                            // 播放音效 (選用)
                            // SC->play("./assets/sound/coin.wav", ALLEGRO_PLAYMODE_ONCE);
                                
                            break;
                        }
                        
                    }
                }

                // 暫停鍵
                if(DC->key_state[ALLEGRO_KEY_P] && !DC->prev_key_state[ALLEGRO_KEY_P]) {
                    SC->toggle_playing(bgm_instance);
                    state = STATE::PAUSE;
                }

                // 滑鼠左鍵「按下瞬間」才判定
                if (DC->mouse_state[1] && !DC->prev_mouse_state[1]) {

                    Point mp(mouse_x, mouse_y);   // 目前滑鼠座標

                    // 逐一檢查場上所有 Special（Typhoon / Wind）
                    for (Special* sp : DC->specials) {

                        // -------------------------------------------------------
                        // (1) 必須是 Typhoon 或 Wind，不是的直接跳過
                        // -------------------------------------------------------
                        SpecialType type = sp->get_type();
                        if (type != SpecialType::TYPHOON &&
                            type != SpecialType::WIND)
                            continue;


                        // -------------------------------------------------------
                        // (2) 轉型成對應的子類別（方便讀取 dying / death_timer）
                        // -------------------------------------------------------
                        SpecialTyphoon* t = nullptr;
                        SpecialWind*    w = nullptr;

                        if (type == SpecialType::TYPHOON)
                            t = static_cast<SpecialTyphoon*>(sp);
                        else
                            w = static_cast<SpecialWind*>(sp);


                        // -------------------------------------------------------
                        // (3) 必須處於死亡動畫（dying == true）
                        // -------------------------------------------------------
                        bool dying = (type == SpecialType::TYPHOON) ? t->dying : w->dying;
                        if (!dying)
                            continue;


                        // -------------------------------------------------------
                        // (4) 必須是死亡圖片 2.png（你定義 death_frame_index = 2）
                        // -------------------------------------------------------
                        if (sp->death_frame_index != 2)
                            continue;


                        // -------------------------------------------------------
                        // (5) 滑鼠點擊是否落在該 Special 的 hitbox 上
                        // -------------------------------------------------------
                        if (!sp->shape->overlap(mp))
                            continue;


                        // =======================================================
                        // (6) 命中！根據不同類型進行計數 + 移除死亡動畫
                        // =======================================================

                        if (type == SpecialType::TYPHOON) {

                            DC->player->typhoon_count++;   // ★ Typhoon +1
                            debug_log("[Typhoon] Clicked! Count = %d\n",
                                    DC->player->typhoon_count);

                            //t->death_timer = 0;            // 讓 Level.cpp 立即移除
                        }
                        else { // WIND

                            DC->player->wind_count++;      // ★ Wind +1
                            debug_log("[Wind] Clicked! Count = %d\n",
                                    DC->player->wind_count);

                            //w->death_timer = 0;            // 讓 Level.cpp 立即移除
                        }

                        sp->flying_to_ui = true;
                        sp->fly_timer = 30;  // 飛 20 frame，可以調整速度

                        // 設定起點
                        sp->start_x = sp->shape->center_x();
                        sp->start_y = sp->shape->center_y();
                        sp->fly_x = sp->start_x;
                        sp->fly_y = sp->start_y;

                        // 設定終點（你指定的位置）
                        if (sp->get_type() == SpecialType::TYPHOON) {
                            sp->ui_target_x = 65;
                            sp->ui_target_y = 570;
                        }
                        else { // WIND
                            sp->ui_target_x = 180;
                            sp->ui_target_y = 570;
                        }

                        // -------------------------------------------------------
                        // (7) 為避免多重點擊命中多個 Special → break
                        // -------------------------------------------------------
                        break;
                    }
                }
                
                // ==========================================
                // ★★★ 修改：生存模式勝利判斷 ★★★
                // ==========================================
                
                // 1. 倒數計時
                if (level_timer > 0) {
                    level_timer--;
                } 
                else {
                    
                    // 2. 時間到了！(level_timer <= 0)
                    // 如果玩家還活著 (HP > 0)，就是贏了
                    if (DC->player->HP > 0) {
                        //printf("final coin:%d\n", DC->player->coin);
                        
                        
                        is_win = true;
                        DC->player->add_record(DC->player->coin, DC->player->current_kills, is_win);
                        
                        // 停音樂
                        if (level_bgm) {
                            SC->stop(level_bgm);
                            level_bgm = nullptr;
                        }
                        
                        // 紀錄成績 (如果你有做排行榜功能)
                        // printf("final coin:%d\n", DC->player->coin);
                        // DC->player->add_record(DC->player->coin, DC->player->current_kills, true);

                        state = STATE::RESULT;
                    }
                }

                // --- 失敗判斷 ---
                if(DC->player->HP <= 0) {
                    is_win = false;
                    SC->stop(bgm_instance);
                    bgm_instance = nullptr;
                    DC->player->add_record(DC->player->coin, DC->player->current_kills, is_win);

                    if (DC->player->coin > DC->player->high_score) {
                        DC->player->high_score = DC->player->coin;
                        // ★★★ 紀錄成績 ★★★
                        // DC->player->add_record(DC->player->coin, DC->player->current_kills, true);
                    }
                    // 清除場上的東西
                    //delete_all();

                    
                    // ★★★★★★★★★★★★★★★★★

                    state = STATE::RESULT;
                }
            }
            if(current_level == 2) {
                // 1. 倒數計時
                if (level_timer > 0) {
                    level_timer--;
                } 
                else {
                    // 2. 時間到了！(level_timer <= 0)
                    
                    // --- 失敗判斷 ---
                    if(DC->player->HP > 0) {
                        is_win = false;
                        SC->stop(bgm_instance);
                        bgm_instance = nullptr;

                        // 停音樂
                        if (level_bgm) {
                            SC->stop(level_bgm);
                            level_bgm = nullptr;
                        }

                        state = STATE::RESULT;
                    }
                }
                if(DC->player->HP <= 0) {
                    is_win = true;
                    SC->stop(bgm_instance);
                    bgm_instance = nullptr;
                    // 停音樂
                    if (level_bgm) {
                        SC->stop(level_bgm);
                        level_bgm = nullptr;
                    }

                    state = STATE::RESULT;
                }
            }
            
            // 遊戲邏輯更新
            DC->player->update();
            DC->level->update();

            for (auto* r : DC->robots)
                r->update();

            for (auto* b : DC->beams)
                b->update();

            for (auto* s : DC->students)
                s->update();

            for (auto* ss : DC->specials)
                ss->update();
            
            for (auto* star : DC->stars)
                star->update();

            OC->update();
            ui->update();

            // ★★★ 新增這段：統一清理死掉的學生 ★★★
            // 這是最安全的做法，防止在迴圈中刪除導致崩潰
            for(auto it = DC->students.begin(); it != DC->students.end(); ) {
                if((*it)->HP <= 0) {
                    delete *it;
                    DC->player->current_kills++;
                    it = DC->students.erase(it);
                } else {
                    ++it;
                }
            }

            // 清理死掉的機器人
            for(auto it = DC->robots.begin(); it != DC->robots.end(); ) {
                if((*it)->HP <= 0) {
                    
                    // ★★★ 新增：反向尋找這個機器人是在哪個 Block 上，並清空它 ★★★
                    // 這裡需要一個簡單的方法找到 Block。
                    // 方法 A: 機器人身上存 Block 指標。
                    // 方法 B: 用機器人座標反算 Col/Row。
                    
                    int col = ((*it)->shape->center_x() - (GRID_START_X - GRID_W/2)) / GRID_W;
                    int row = ((*it)->shape->center_y() - (GRID_START_Y - GRID_H/2)) / GRID_H;
                    Block* b = DC->level->get_block(col, row);
                    if(b) b->clear_robot(); // 告訴格子：我走了，你可以放新人了

                    delete *it;
                    it = DC->robots.erase(it);
                } else {
                    ++it;
                }
            }
            break;
        }



        // 6. 暫停
        case STATE::PAUSE: {
            if(DC->key_state[ALLEGRO_KEY_P] && !DC->prev_key_state[ALLEGRO_KEY_P]) {
                SC->toggle_playing(bgm_instance);
                state = STATE::LEVEL;
            }
            break;
        }

        // 7. 結算畫面 (流程圖的精髓)
        case STATE::RESULT: {
            // (A) 如果正在播選單音樂，先停掉
            if (level_bgm) {
                SC->stop(level_bgm);
                level_bgm = nullptr;
            }

            if(is_win){
                if(!win_bgm){
                    win_bgm = SC->play(win_sound_path, ALLEGRO_PLAYMODE_LOOP);
                }
            } else {
                if(!lose_bgm){
                    lose_bgm = SC->play(lose_sound_path, ALLEGRO_PLAYMODE_LOOP);
                }
            }
        
			

            if(is_clicked) {
                // 取得按鈕的圖片資訊
                ALLEGRO_BITMAP *btn_img = ImageCenter::get_instance()->get(menu_button_img_path);
                int source_w = al_get_bitmap_width(btn_img);
                int source_h = al_get_bitmap_height(btn_img);

                // 計算按鈕位置 (跟 game_draw 一樣)
                const int target_w = 150; 
                const int target_h = (int)((float)source_h * ((float)target_w / source_w));
                int btn_x = DC->window_width - target_w - 20;
                int btn_y = DC->window_height - target_h - 20;

                // 判斷是否點擊了「回主選單按鈕」
                if (mouse_x >= btn_x && mouse_x <= btn_x + target_w &&
                    mouse_y >= btn_y && mouse_y <= btn_y + target_h) {

                    if(is_win){
                        if(win_bgm){
                            printf("[win] is_win:%d\n", is_win);
                            SC->stop(win_bgm);
                            win_bgm = nullptr;
                        }
                    } else {
                        if(lose_bgm){
                            printf("[lose] is_win:%d\n", is_win);
                            SC->stop(lose_bgm);
                            lose_bgm = nullptr;
                        }
                    }
                    
                    // 點到了！直接回主選單
                    state = STATE::MENU;
                    delete_all();
                } 
            
            }
            break;
        }

        // 8. 結束程式
        case STATE::END: {
            return false;
        }
    }

    // 處理全域的更新 (Sound)
    SC->update();

    // 更新上一幀的輸入狀態
    memcpy(DC->prev_key_state, DC->key_state, sizeof(DC->key_state));
    memcpy(DC->prev_mouse_state, DC->mouse_state, sizeof(DC->mouse_state));
    return true;
}

/**
 * @brief Draw the whole game and objects.
 */
void Game::game_draw() {
    DataCenter *DC = DataCenter::get_instance();
    OperationCenter *OC = OperationCenter::get_instance();
    FontCenter *FC = FontCenter::get_instance();
    ImageCenter *IC = ImageCenter::get_instance();


    int win_w = DC->window_width;
    int win_h = DC->window_height;

    // 1. 確保 scene_buffer 存在且大小正確
    if (!scene_buffer ||
        al_get_bitmap_width(scene_buffer)  != win_w ||
        al_get_bitmap_height(scene_buffer) != win_h) {

        if (scene_buffer) {
            al_destroy_bitmap(scene_buffer);
        }
        scene_buffer = al_create_bitmap(win_w, win_h);
    }
    
    // 2. 把接下來所有的畫圖「暫時改成畫在 scene_buffer 上」
    ALLEGRO_BITMAP* old_target = al_get_target_bitmap();
    al_set_target_bitmap(scene_buffer);

    // 清空的是 buffer，不是螢幕
    al_clear_to_color(al_map_rgb(100, 100, 100));

    // 1. 清空畫面
    //al_clear_to_color(al_map_rgb(100, 100, 100));




    // 2. 根據狀態畫圖
    switch(state) {
        case STATE::SPLASH:
            // 畫進場 LOGO
            al_draw_text(FC->caviar_dreams[FontSize::LARGE], al_map_rgb(255, 255, 255),
                         DC->window_width/2, DC->window_height/2, ALLEGRO_ALIGN_CENTRE, "MY AWESOME GAME");
            break;

        case STATE::MENU: {
            // 預設畫原始背景
            const char* target_img_path = menu_img_path;

            // 根據 hovered_button 狀態切換圖片路徑
            switch(hovered_button) {
                case MenuButton::START:
                    target_img_path = menu_start_glow_path; // 記得確認 Game.cpp 最上面有定義這個路徑
                    break;
                case MenuButton::EXIT:
                    target_img_path = menu_exit_glow_path;
                    break;
                case MenuButton::NTU:
                    target_img_path = menu_ntu_glow_path;
                    break;
                case MenuButton::CUTLETRUSH:
                    target_img_path = menu_cutletrush_glow_path;
                    break;
                case MenuButton::NONE:
                default:
                    target_img_path = menu_img_path;
                    break;
            }

            // 取得圖片並繪製
            ALLEGRO_BITMAP *bg_to_draw = IC->get(target_img_path);
            if (bg_to_draw) {
                al_draw_scaled_bitmap(
                    bg_to_draw,
                    0, 0, al_get_bitmap_width(bg_to_draw), al_get_bitmap_height(bg_to_draw),
                    0, 0, DC->window_width, DC->window_height,
                    0
                );
            }
            break;
        }

        case STATE::SETTING: {
            // 1. 畫背景圖 (setting.jpg)
            
            const char* setting_bg = setting_img_path;

            switch(hovered_settingbutton) {
                case SettingButton::MENUBOTTON:
                    setting_bg = setting_glow_img_path;
                    break;
                case SettingButton::NONE:
                    setting_bg = setting_img_path;
                    break;
                default:
                    setting_bg = setting_img_path;
                    break;
            }

            ALLEGRO_BITMAP *bg_to_draw = IC->get(setting_bg);

            if (bg_to_draw) {
                al_draw_scaled_bitmap(
                    bg_to_draw,
                    0, 0, al_get_bitmap_width(bg_to_draw), al_get_bitmap_height(bg_to_draw),
                    0, 0, DC->window_width, DC->window_height,
                    0
                );
            } else {
                // 如果找不到圖，就畫灰色底當備案
                al_clear_to_color(al_map_rgb(50, 50, 50));
            }
            

            // 2. 畫音量條 (畫在右邊木板上)
            
            // (A) 軌道底色 (深褐色，配合木頭風格)
            al_draw_filled_rectangle(vol_track_rect.x1, vol_track_rect.y1, vol_track_rect.x2, vol_track_rect.y2, al_map_rgb(60, 40, 20));
            
            // (B) 已填滿部分 (亮橘色或黃色)
            float filled_w = VOL_TRACK_W * BGM_volume;
            al_draw_filled_rectangle(VOL_TRACK_X, VOL_TRACK_Y, VOL_TRACK_X + filled_w, VOL_TRACK_Y + VOL_TRACK_H, al_map_rgb(255, 180, 0));

            // (C) 滑塊 (白色方塊)
            al_draw_filled_rectangle(vol_knob_rect.x1, vol_knob_rect.y1, vol_knob_rect.x2, vol_knob_rect.y2, al_map_rgb(255, 255, 230));
            // 幫滑塊加個黑框比較明顯
            al_draw_rectangle(vol_knob_rect.x1, vol_knob_rect.y1, vol_knob_rect.x2, vol_knob_rect.y2, al_map_rgb(0, 0, 0), 2);

            // 3. 畫文字提示
            // 標題 "Music Volume"
            al_draw_text(FC->caviar_dreams[FontSize::MEDIUM], al_map_rgb(60, 40, 20), // 深褐色字
                         VOL_TRACK_X + VOL_TRACK_W/2, VOL_TRACK_Y - 40, ALLEGRO_ALIGN_CENTRE, "Music Volume");
            
            // 顯示趴數 (例如: 80%)
            char vol_str[10];
            sprintf(vol_str, "%d%%", (int)(BGM_volume * 100));
            al_draw_text(FC->caviar_dreams[FontSize::MEDIUM], al_map_rgb(60, 40, 20), 
                         VOL_TRACK_X + VOL_TRACK_W + 40, VOL_TRACK_Y - 10, ALLEGRO_ALIGN_CENTRE, vol_str);


            auto draw_btn = [&](Rectangle &r, const char* text, bool active) {
                ALLEGRO_COLOR color = active ? al_map_rgb(255, 200, 40) : al_map_rgb(240, 240, 240);
                ALLEGRO_COLOR border = active ? al_map_rgb(255, 120, 0) : al_map_rgb(50, 50, 50);

                al_draw_filled_rectangle(r.x1, r.y1, r.x2, r.y2, color);
                al_draw_rectangle(r.x1, r.y1, r.x2, r.y2, border, 3);

                al_draw_text(FC->caviar_dreams[FontSize::MEDIUM],
                            al_map_rgb(0,0,0),
                            (r.x1 + r.x2) / 2,
                            r.y1 + 15,
                            ALLEGRO_ALIGN_CENTRE,
                            text);
            };

            // 根據目前畫質畫三個按鈕（高亮目前選中的）
            draw_btn(graphics_low_rect,  "Low Quality",    DC->graphics_quality == GraphicsQuality::LOW);
            draw_btn(graphics_med_rect,  "Medium Quality", DC->graphics_quality == GraphicsQuality::MEDIUM);
            draw_btn(graphics_high_rect, "High Quality",   DC->graphics_quality == GraphicsQuality::HIGH);


            // 4. 返回按鈕 -> debug 用
            //al_draw_rectangle(back_btn_rect.x1, back_btn_rect.y1, back_btn_rect.x2, back_btn_rect.y2, al_map_rgb(255, 0, 0), 2);
            //al_draw_rectangle(back_btn_rect.x1, back_btn_rect.y1, back_btn_rect.x2, back_btn_rect.y2, al_map_rgb(255, 0, 0), 3);
            //al_draw_text(FC->caviar_dreams[FontSize::MEDIUM], al_map_rgb(255, 255, 255), 
            //             (back_btn_rect.x1 + back_btn_rect.x2)/2, back_btn_rect.y1 + 10, 
            //             ALLEGRO_ALIGN_CENTRE, "MAIN MENU");
            
            break;
        }

        case STATE::LEVEL_SELECT:
        {
            ALLEGRO_BITMAP *bg_to_draw = IC->get(level_select_img_path);
            al_draw_scaled_bitmap(
                    bg_to_draw,
                    0, 0, al_get_bitmap_width(bg_to_draw), al_get_bitmap_height(bg_to_draw),
                    0, 0, DC->window_width, DC->window_height,
                    0
            );
            break;
        }

        case STATE::INSTRUCTION:
        {
            ALLEGRO_BITMAP *bg_to_draw = IC->get(news_img_path);
            al_draw_scaled_bitmap(
                    bg_to_draw,
                    0, 0, al_get_bitmap_width(bg_to_draw), al_get_bitmap_height(bg_to_draw),
                    0, 0, DC->window_width, DC->window_height,
                    0
            );
            break;
        }

        case STATE::LEVEL:
        case STATE::PAUSE:
        {
            const char* current_bg_path = level_background_img_path; // 預設 Level 1
            
            if (current_level == 2) {
                current_bg_path = level_background_img_Newpath; // Level 2 換這張
            }


            ALLEGRO_BITMAP *level_bg = IC->get(current_bg_path);
            int win_w = DC->window_width;
            int win_h = DC->window_height;

        // // 先記住當前 target
        // ALLEGRO_BITMAP* old_target = al_get_target_bitmap();

        // if (DC->graphics_quality == GraphicsQuality::LOW)
        // {
        //     int small_w = win_w / 6;
        //     int small_h = win_h / 6;

        //     ALLEGRO_BITMAP* small = al_create_bitmap(small_w, small_h);

        //     // 1. 小圖繪製
        //     al_set_target_bitmap(small);
        //     al_draw_scaled_bitmap(
        //         level_bg,
        //         0, 0, al_get_bitmap_width(level_bg), al_get_bitmap_height(level_bg),
        //         0, 0, small_w, small_h,
        //         0
        //     );

        //     // 2. 回到 backbuffer，把小圖放大回螢幕
        //     al_set_target_backbuffer(display);
        //     al_draw_scaled_bitmap(
        //         small,
        //         0, 0, small_w, small_h,
        //         0, 0, win_w, win_h,
        //         0
        //     );

        //     al_destroy_bitmap(small);
        // }
        // else
        // {
        //     // Medium & High 照常畫
        //     al_set_target_backbuffer(display);
        //     al_draw_scaled_bitmap(
        //         level_bg,
        //         0, 0, al_get_bitmap_width(level_bg), al_get_bitmap_height(level_bg),
        //         0, 0, win_w, win_h,
        //         0
        //     );
        // }

        // // ★★★★★ 最重要：恢復原本 target（一定要有）★★★★★
        // al_set_target_bitmap(old_target);



            // ALLEGRO_BITMAP *level_bg = IC->get(current_bg_path);
            if (level_bg) {
                al_draw_scaled_bitmap(
                    level_bg,
                    0, 0, al_get_bitmap_width(level_bg), al_get_bitmap_height(level_bg), // 來源
                    0, 0, DC->window_width, DC->window_height, // ★★★ 目標：直接強制填滿視窗
                    0
                );
            }

            // 畫出typhoon的圖
            for (auto &t : ty_list) {

                ALLEGRO_BITMAP* bmp = (t.frame == 0 ? ty_frame0 : ty_frame1);

                int w = al_get_bitmap_width(bmp);
                int h = al_get_bitmap_height(bmp);
                float scale = 0.15f;

                al_draw_scaled_bitmap(
                    bmp,
                    0, 0, w, h,
                    t.x,
                    t.y,
                    w * scale,
                    h * scale,
                    0
                );
            }

            // ======================================================
            // ★★★ Wind Reveal Animation — 動態裁切＋往右飛 ★★★
            // ======================================================
            for (auto &w : wind_list) {

                if (!wind_img) continue;

                int full_w = al_get_bitmap_width(wind_img);
                int full_h = al_get_bitmap_height(wind_img);

                int reveal_w = full_w;

                // reveal_step：0=1/3，1=1/2，2=全幅
                if (w.reveal_step == 0)
                    reveal_w = full_w / 3;
                else if (w.reveal_step == 1)
                    reveal_w = full_w / 2;
                else
                    reveal_w = full_w;

                float scale = 0.25f;

                // ★★★ 動態裁切後畫出風動畫
                al_draw_scaled_bitmap(
                    wind_img,
                    0, 0, reveal_w, full_h,     // ← 裁切左邊 reveal_w 寬度
                    w.x, w.y,                   // ← 放在螢幕位置
                    reveal_w * scale, full_h * scale,
                    0
                );
            }


            //al_draw_bitmap(background, 0, 0, 0); 
            DC->level->draw();
            //DC->hero->draw();

            for (auto* r : DC->robots)
    			r->draw();

            for (auto* b : DC->beams)
                b->draw();

            for (auto* s : DC->students)
    			s->draw();

            for (auto* star : DC->stars)
                star->draw();
                

            
            // 畫出右下角的 Menu 按鈕
            ALLEGRO_BITMAP *btn_img = IC->get(menu_button_img_path);
            if (btn_img) {
                al_draw_scaled_bitmap(
                    btn_img, 
                    0, 0, al_get_bitmap_width(btn_img), al_get_bitmap_height(btn_img),
                    level_menu_btn_rect.x1, level_menu_btn_rect.y1, 
                    level_menu_btn_rect.x2 - level_menu_btn_rect.x1, 
                    level_menu_btn_rect.y2 - level_menu_btn_rect.y1, 
                    0
                );
            }

            OC->draw();
            ui->draw();

            // ==========================================
            // ★★★ 畫出剩餘時間 ★★★
            // ==========================================
            // 將 frame 換算成秒數 (無條件進位)
            int seconds_left = (int)ceil(level_timer / 60.0);
            
            // 畫在螢幕正上方
            al_draw_textf(
                FC->courier_new[FontSize::MEDIUM], // 用小字體
                al_map_rgb(255, 255, 255),        // 白色字
                DC->window_width / 2 + 275,       // 靠右
                20,                               // 距離頂部 20px
                ALLEGRO_ALIGN_CENTRE, 
                "TIME:%d", 
                seconds_left
            );
            
            // (選用) 如果剩 5 秒，字變紅色增加緊張感
            if (seconds_left <= 5) {
                 al_draw_textf(FC->courier_new[FontSize::MEDIUM], al_map_rgb(255, 50, 50), 
                 DC->window_width / 2 + 275, 20, ALLEGRO_ALIGN_CENTRE, "TIME:%d", seconds_left);
            }

            if (is_return_menu_active) {
                // (A) 畫半透明黑底 (讓背景變灰暗)
                al_draw_filled_rectangle(0, 0, DC->window_width, DC->window_height, al_map_rgba(0, 0, 0, 150));

                // (B) 畫確認視窗圖片 (置中)
                ALLEGRO_BITMAP *box_img = IC->get(return_menu_img_path);
                if (box_img) {
                    // 使用我們在上方定義的座標和寬高常數
                    al_draw_scaled_bitmap(
                        box_img,
                        0, 0, al_get_bitmap_width(box_img), al_get_bitmap_height(box_img),
                        150, 190, 500, 200,
                        0
                    );
                }

                // (C) Debug 用：畫出 YES/NO 的判定框 (確認位置用，調好後可註解掉)
                //al_draw_rectangle(return_yes_rect.x1, return_yes_rect.y1, return_yes_rect.x2, return_yes_rect.y2, al_map_rgb(0, 255, 0), 3);
                //al_draw_rectangle(return_no_rect.x1, return_no_rect.y1, return_no_rect.x2, return_no_rect.y2, al_map_rgb(255, 0, 0), 3);
            }
            
            if(state == STATE::PAUSE) {
                al_draw_filled_rectangle(0, 0, DC->window_width, DC->window_height, al_map_rgba(50, 50, 50, 64));
                al_draw_text(FC->caviar_dreams[FontSize::LARGE], al_map_rgb(255, 255, 255),
                             DC->window_width/2, DC->window_height/2, ALLEGRO_ALIGN_CENTRE, "PAUSED");
            }
            break;
        }
        case STATE::RESULT: {
            // 1. 畫背景 (Win/Lose)
            ALLEGRO_BITMAP *result_img = nullptr;
            if(is_win) {
                result_img = IC->get(win_img_path);
            } else {
                result_img = IC->get(lose_img_path);
            }

            if(result_img) {
                al_draw_scaled_bitmap(
                    result_img,
                    0, 0, al_get_bitmap_width(result_img), al_get_bitmap_height(result_img),
                    0, 0, DC->window_width, DC->window_height,
                    0
                );
            }

            // 2. 畫出「回主選單」按鈕
            ALLEGRO_BITMAP *btn_img = IC->get(menu_button_img_path);
            int source_w = al_get_bitmap_width(btn_img);
            int source_h = al_get_bitmap_height(btn_img);
            const int target_w = 150; 
            const int target_h = (int)((float)source_h * ((float)target_w / source_w));
            int btn_x = DC->window_width - target_w - 20;
            int btn_y = DC->window_height - target_h - 20;
            
            al_draw_scaled_bitmap(
                btn_img,
                0, 0, source_w, source_h,
                btn_x, btn_y, target_w, target_h,
                0
            );

           

            

            if(current_level == 1){

                // ==========================================
                // ★★★ 新增：半透明黑底 (讓文字更清楚) ★★★
                // ==========================================
                
                // 設定黑底的大小與位置 (根據文字位置調整)
                // 這裡假設文字集中在螢幕中央，寬度 400，高度 200
                int cx = DC->window_width / 2;
                int cy = DC->window_height / 2;
                
                int box_w = 400; // 黑框寬度
                int box_h = 220; // 黑框高度
                
                // 畫出半透明圓角矩形 (圓角半徑 10, 顏色黑色, 透明度約 60%)
                al_draw_filled_rounded_rectangle(
                    cx - box_w/2, cy - 100,      // 左上角 (y 稍微往上提一點包住 High Score)
                    cx + box_w/2, cy - 100 + box_h, // 右下角
                    10, 10,                      // 圓角半徑
                    al_map_rgba(0, 0, 0, 160)    // 黑色半透明
                );
                
                // (選用) 加個白框讓它更有質感
                al_draw_rounded_rectangle(
                    cx - box_w/2, cy - 100, 
                    cx + box_w/2, cy - 100 + box_h, 
                    10, 10, 
                    al_map_rgb(255, 255, 255), 2
                );
                // ==========================================
                // ★★★ 新增：顯示結算數據 ★★★
                // ==========================================
                
                // 設定文字顏色 (根據背景深淺調整，這裡假設用白色字)
                ALLEGRO_COLOR text_color = al_map_rgb(255, 255, 255);
                // 或是加個陰影/外框顏色讓字更清楚
                ALLEGRO_COLOR shadow_color = al_map_rgb(0, 0, 0);

                cx = DC->window_width / 2;
                cy = DC->window_height / 2; // 畫面中心點

                // 1. 顯示當前分數 (Coin)
                al_draw_textf(FC->courier_new[FontSize::LARGE], shadow_color, cx + 2, cy - 60 + 2, ALLEGRO_ALIGN_CENTRE, "Final Score: %d", DC->player->coin);
                al_draw_textf(FC->courier_new[FontSize::LARGE], text_color,   cx,     cy - 60,     ALLEGRO_ALIGN_CENTRE, "Final Score: %d", DC->player->coin);

                // 2. 顯示最高分數 (High Score)
                al_draw_textf(FC->caviar_dreams[FontSize::MEDIUM], shadow_color, cx + 2, cy - 10 + 2, ALLEGRO_ALIGN_CENTRE, "High Score: %d", DC->player->high_score);
                al_draw_textf(FC->caviar_dreams[FontSize::MEDIUM], text_color,   cx,     cy - 10,     ALLEGRO_ALIGN_CENTRE, "High Score: %d", DC->player->high_score);

                // 3. 顯示剩餘血量 (HP)
                // 只有贏的時候顯示剩餘 HP 才有意義，輸了通常是 0 或負的
                if (is_win) {
                    al_draw_textf(FC->caviar_dreams[FontSize::MEDIUM], shadow_color, cx + 2, cy + 30 + 2, ALLEGRO_ALIGN_CENTRE, "HP Remaining: %d", DC->player->HP);
                    al_draw_textf(FC->caviar_dreams[FontSize::MEDIUM], al_map_rgb(255, 100, 100), cx, cy + 30, ALLEGRO_ALIGN_CENTRE, "HP Remaining: %d", DC->player->HP);
                }
                // ==========================================
            }

            break;
        }
            
        case STATE::END:
            break;
    }

    // 4. 畫質處理：把 scene_buffer 貼到「真正的螢幕背面」
    al_set_target_backbuffer(display);

    if (DC->graphics_quality == GraphicsQuality::LOW) {
        // ★ 低畫質：先縮到 1/4 大，再放大回去 → 超明顯的糊畫質
        int small_w = win_w / 4;
        int small_h = win_h / 4;
        ALLEGRO_BITMAP* small = al_create_bitmap(small_w, small_h);

        // 4-1 把整個畫面縮小到 small
        al_set_target_bitmap(small);
        al_draw_scaled_bitmap(
            scene_buffer,
            0, 0, win_w, win_h,
            0, 0, small_w, small_h,
            0
        );

        // 4-2 再把 small 放大回螢幕
        al_set_target_backbuffer(display);
        al_draw_scaled_bitmap(
            small,
            0, 0, small_w, small_h,
            0, 0, win_w, win_h,
            0
        );

        al_destroy_bitmap(small);
    }
    else if (DC->graphics_quality == GraphicsQuality::MEDIUM) {
        // ★ 中畫質：縮到 1/2 大再放大 → 稍微糊、介於中間
        int small_w = win_w / 2;
        int small_h = win_h / 2;
        ALLEGRO_BITMAP* small = al_create_bitmap(small_w, small_h);

        al_set_target_bitmap(small);
        al_draw_scaled_bitmap(
            scene_buffer,
            0, 0, win_w, win_h,
            0, 0, small_w, small_h,
            0
        );

        al_set_target_backbuffer(display);
        al_draw_scaled_bitmap(
            small,
            0, 0, small_w, small_h,
            0, 0, win_w, win_h,
            0
        );

        al_destroy_bitmap(small);
    }
    else {
        // ★ 高畫質：直接原樣貼到螢幕
        al_draw_scaled_bitmap(
            scene_buffer,
            0, 0, win_w, win_h,
            0, 0, win_w, win_h,
            0
        );
    }

    // 5. 還原原本的 target（通常是 backbuffer，不過這樣寫比較安全）
    al_set_target_bitmap(old_target);

    al_flip_display();
}

Game::~Game() {
    if (scene_buffer) al_destroy_bitmap(scene_buffer);
	if(display) al_destroy_display(display);
	if(timer) al_destroy_timer(timer);
	if(event_queue) al_destroy_event_queue(event_queue);
}


void Game::delete_all() {
    DataCenter *DC = DataCenter::get_instance();

    // 1. 清空所有動態物件

    // 機器人
    for(auto *obj : DC->robots) delete obj;
    DC->robots.clear();

    // 光束/星星
    for(auto *obj : DC->beams) delete obj;
    DC->beams.clear();

    // 學生
    for(auto *obj : DC->students) delete obj;
    DC->students.clear();

    for(auto *obj : DC->specials) delete obj;
    DC->specials.clear();

    //星星
    for(auto *obj : DC->stars) delete obj;
    DC->stars.clear();

    // 2. 重置玩家狀態
    if (DC->player) {
        DC->player->init(); 
    }
    

    // 重製block是否可放robot
    if (DC->level) {
        DC->level->reset();
    }

    // ==========================================
    // ★★★ 新增：重置遊戲勝負與時間變數 ★★★
    // ==========================================
    
    // 1. 重置勝利狀態
    is_win = false; 

    // 2. 重置倒數計時器 (重要！不然下一局會直接時間到而獲勝)
    // 假設是 30 秒，FPS 是 60
    level_timer = 30 * DC->FPS; 

    // ==========================================

    // ==========================================
    // ★★★ 重置輸送帶 ★★★
    // ==========================================
    if (ui) {
        ui->clear_belt(); 
    }
    // ==========================================
    
    debug_log("<Game> All objects deleted and game reset.\n");
}