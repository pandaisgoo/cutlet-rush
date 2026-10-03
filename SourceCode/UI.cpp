#include "UI.h"
#include "Utils.h"
#include "data/DataCenter.h"
#include "data/ImageCenter.h"
#include "data/FontCenter.h"
#include <algorithm>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_ttf.h>
#include "shapes/Point.h"
#include "shapes/Rectangle.h"
#include "Player.h"
#include "Level.h"
#include "robots/Robot.h" // 記得 include Robot

// ★★★ 輸送帶參數設定 ★★★
const int BELT_START_X = 75;   // 輸送帶最左邊（卡片停止點）
const int BELT_Y = 500;         // 輸送帶高度 (螢幕上方)
const int BELT_CARD_W = 60;    // 卡片寬度
const int BELT_CARD_GAP = 10;  // 卡片間距
const int BELT_SPAWN_X = 500; // 卡片出生點 (螢幕右邊外)
const int BELT_MAX_CARDS = 7;  // 最多幾張
const int BELT_SPAWN_DELAY = 120; // 每 2 秒生成一張

// 輸送帶送的學生圖
/*
namespace StudentSetting {
    static constexpr char student_imgs_root_path[static_cast<int>(StudentType::STUDENTTYPE_MAX)][60] = {
        "./assets/image/student/Norm",
        "./assets/image/student/Bicycle",
        "./assets/image/student/Drill",
        "./assets/image/student/Jump",
        "./assets/image/student/Booo"
    };
}
*/
// fixed settings
constexpr char score_table_img_path[] = "./assets/image/score_table.png";
constexpr int tower_img_left_padding = 30;
constexpr int tower_img_top_padding = 30;
// 格子開始的位置
const int GRID_START_X = 175; // 第一格中心 X
const int GRID_START_Y = 140; // 第一格中心 Y
const int GRID_W = 56;        // 格寬
const int GRID_H = 80;        // 格高
const int GRID_COLS = 8;     // 欄數
const int GRID_ROWS = 5;      // 列數

// ★★★ 1. 定義機器人選單的座標常數 (讓 update 和 draw 共用) ★★★
const int ROBOT_MENU_X = 170;
const int ROBOT_MENU_Y = 7;
const int ROBOT_W = 60;
const int ROBOT_H = 60;
const int ROBOT_GAP = 25;

void UI::init() {
    DataCenter *DC = DataCenter::get_instance();
    ImageCenter *IC = ImageCenter::get_instance();
    
    score_table = IC->get(score_table_img_path);

    icon_typhoon = IC->get("./assets/image/typhoon_sign.png");
	icon_wind    = IC->get("./assets/image/wind_sign.png");
    
    //icon_cancel  = IC->get("./assets/image/cross.png");
   

    // initialize robot_dark menu
    for (int i = 0; i < 5; i++) {
        char path[100];
        sprintf(path, "./assets/image/robot_dark/%d.png", i);
        robot_icons.push_back(IC->get(path));
    }
    
    // load robot_light images
    for (int i = 0; i < 5; i++) {
        char path[100];
        sprintf(path, "./assets/image/robot_light/%d.png", i);
        robot_light_icons.push_back(IC->get(path));
    }

    // 初始化變數
    selected_robot = -1;
    on_robot_hover = -1; // 記得在 UI.h 加這個變數

    // 設定價格
    robot_cost = { 5, 10, 10, 30, 20 }; 

    // ★★★ 初始化輸送帶 ★★★
    conveyor_belt.clear();
    conveyor_spawn_timer = 0;
    selected_student_type = -1;

    debug_log("<UI> state: change to HALT\n");
    state = STATE::HALT;
    on_item = -1;
}

void UI::update() {
    DataCenter *DC = DataCenter::get_instance();
    const Point &mouse = DC->mouse;
    
    // 取得滑鼠點擊狀態 (左鍵 & 右鍵)
    bool left_click = DC->mouse_state[1] && !DC->prev_mouse_state[1];
    bool right_click = DC->mouse_state[2] && !DC->prev_mouse_state[2];


    // ==========================================================
    // ★★★ 輸送帶邏輯 (只在 Level 2 運作，且不在放置狀態時) ★★★
    // ==========================================================
    
    // 假設我們用一個變數判斷是否為第二關 (你需要從 DataCenter 拿)
    if (DC->levels == 2) {  
    
        // 1. 生成邏輯 (Spawning)
        if (conveyor_belt.size() < BELT_MAX_CARDS) {
            if (conveyor_spawn_timer > 0) {
                conveyor_spawn_timer--;
            } else {
                // 時間到，隨機生成一個學生
                StudentType type = (StudentType)(rand() % (int)StudentType::STUDENTTYPE_MAX);
                int stu = (int)type;
                if(stu == 0) stu = 1;
                if(stu == 2) stu = 4;
                if(stu == 3) stu = 5;
                
                ConveyorCard card;
                card.type = (StudentType)stu;
                card.x = BELT_SPAWN_X; // 從右邊出生
                card.y = BELT_Y;
                
                // 取得圖片 (這裡假設用走路圖的 0.png 當圖示)
                char buffer[100];
                sprintf(buffer, "%s/%d.png", 
                    StudentSetting::student_imgs_root_path[stu], 0);
                card.img = ImageCenter::get_instance()->get(buffer);

                conveyor_belt.push_back(card);
                conveyor_spawn_timer = BELT_SPAWN_DELAY;
            }
        }

        // 2. 移動與目標計算 (Moving)
        // 規則：第 0 張卡停在 BELT_START_X，第 i 張停在 i-1 張的後面
        float next_target_x = BELT_START_X;
        
        for (auto &card : conveyor_belt) {
            card.target_x = next_target_x;
            
            // 平滑移動 (Lerp)
            if (std::abs(card.x - card.target_x) > 1.0f) {
                card.x += (card.target_x - card.x) * 0.1f;
            } else {
                card.x = card.target_x;
            }

            // 更新下一個目標位置
            next_target_x += (BELT_CARD_W + BELT_CARD_GAP);
        }

        // 3. 點擊拿取 (Picking)
        // 只有在 HALT 或 HOVER 狀態下可以拿
        if (state == STATE::HALT || state == STATE::HOVER) {
            int clicked_index = -1;
            
            for (int i = 0; i < conveyor_belt.size(); ++i) {
                ConveyorCard &card = conveyor_belt[i];
                Rectangle rect{card.x, card.y, card.x + BELT_CARD_W, card.y + BELT_CARD_W}; // 假設正方形
                
                // 滑鼠指到卡片
                if (mouse.overlap(rect)) {
                    state = STATE::HOVER; // 切換狀態避免跟其他 UI 衝突
                    
                    if (left_click) {
                        clicked_index = i;
                        break;
                    }
                }
            }

            // 如果有點到卡片
            if (clicked_index != -1) {
                // 記住選了什麼學生
                selected_student_type = (int)conveyor_belt[clicked_index].type;
                
                // 從輸送帶移除 (後面的卡片會自動補上)
                conveyor_belt.erase(conveyor_belt.begin() + clicked_index);
                
                // 進入放置狀態
                state = STATE::PLACE; // 或者直接進 PLACE
                debug_log("Picked student type: %d\n", selected_student_type);
            }
        }

        // 4. 放置邏輯 (STATE::PLACE)
        if (state == STATE::PLACE) {
            
            // (A) 右鍵取消 (保持不變)
            if (right_click) {
                selected_student_type = -1;
                state = STATE::HALT;
                return; 
            }

            // (B) 左鍵放置功能
            if (left_click && selected_student_type != -1) {
                
                // 1. 定義地圖參數
                //const int GRID_W = 40; 
                //const int GRID_H = 40;
                //const int GRID_START_X = 50; 
                //const int GRID_START_Y = 50;
                //const int MAP_COLS = 15;
                //const int MAP_ROWS = 12;
                // 1. 定義參數 (需與 update 一致)
                const int GRID_W = 56; 
                const int GRID_H = 80;
                const int GRID_START_X = 50; 
                const int GRID_START_Y = 110;
                const int MAP_COLS = 12;
                const int MAP_ROWS = 5;
                const int COL_LIMIT = 8; // ★ 右邊限制
                

                // 2. 計算滑鼠點擊在哪個格子
                // 加上偏移量 (GRID_W/2) 讓點擊判定更直覺
                int col = (mouse.x - (GRID_START_X - GRID_W/2)) / GRID_W;
                int row = (mouse.y - (GRID_START_Y - GRID_H/2)) / GRID_H;

                // 3. 檢查是否在合法範圍 (右邊區塊 COL_LIMIT ~ MAP_COLS)
                if (col >= COL_LIMIT && col < MAP_COLS && row >= 0 && row < MAP_ROWS) {
                    
                    std::vector<Point> path;
                    
                    // 算出該格子的「中心點像素座標」
                    int spawn_x = GRID_START_X + col * GRID_W + GRID_W / 2;
                    int spawn_y = GRID_START_Y + row * GRID_H + GRID_H / 2;
                    path.push_back(Point(spawn_x, spawn_y)); // 起點
                    path.push_back(Point(0, spawn_y));   // 終點 (往左走)
                    debug_log("[DEBUG] Spawn X: %d, Spawn Y: %d (Grid: %d, %d)\n", spawn_x, spawn_y, col, row);

                    // 生成學生
                    DC->students.emplace_back(
                        Student::create_student((StudentType)selected_student_type, path)
                    );
                    
                    debug_log("<UI> Spawn Success at Grid(%d, %d)\n", col, row);

                    // ============================================================
                    // ★★★ 關鍵修正：生成後立刻強制覆寫座標 (只改 UI.cpp 的絕招) ★★★
                    // ============================================================
                    Student* s = Student::create_student((StudentType)selected_student_type, path);
                    DC->students.emplace_back(s);
                    
                    // 1. 算出你想要的「完美像素座標」 (跟你畫紅框的公式一樣)
                    int perfect_x = GRID_START_X + col * GRID_W + GRID_W / 2;
                    int perfect_y = GRID_START_Y + row * GRID_H + GRID_H / 2;

                    // 2. 強制把學生移過去 (覆蓋掉 Student 建構子算出來的結果)
                    // 注意：這裡假設 s->shape 是 public 的，或是你有權限存取 Object 的 shape
                    if (s->shape) {
                        s->shape->update_center_x(perfect_x);
                        s->shape->update_center_y(perfect_y);
                    }

                    // 放置完成，重置狀態
                    selected_student_type = -1;
                    state = STATE::HALT;
                } 

                else {
                    debug_log("<UI> Invalid Position! Must be Col >= %d\n", COL_LIMIT);
                }
            }
        }
    }

    if(DC->levels == 1){
        switch(state) {
            case STATE::HALT: {
                // 檢查滑鼠是否指到【機器人選單】
                // 我們不再檢查 tower_items，只檢查 robot icons
                for(int i = 0; i < 5; ++i) {
                    // 計算第 i 個機器人卡片的座標 (使用常數)
                    int x = ROBOT_MENU_X + i * (ROBOT_W + ROBOT_GAP);
                    int y = ROBOT_MENU_Y;
                    
                    // 檢查滑鼠是否重疊
                    if(mouse.overlap(Rectangle{x, y, x + ROBOT_W, y + ROBOT_H})) {
                        on_robot_hover = i; // 記錄指到第幾隻機器人
                        debug_log("<UI> state: change to HOVER (Robot %d)\n", i);
                        state = STATE::HOVER;
                        break;
                    }
                }
                break;
            } 
            
            case STATE::HOVER: {
                // 處理機器人選單的互動
                if (on_robot_hover != -1) {
                    int x = ROBOT_MENU_X + on_robot_hover * (ROBOT_W + ROBOT_GAP);
                    int y = ROBOT_MENU_Y;

                    // 1. 如果滑鼠移開了選單 -> 回到 HALT
                    if(!mouse.overlap(Rectangle{x, y, x + ROBOT_W, y + ROBOT_H})) {
                        on_robot_hover = -1;
                        state = STATE::HALT;
                        break;
                    }
                    
                    // 2. 如果點擊左鍵 -> 嘗試購買
                    if(left_click) {
                        int price = robot_cost[on_robot_hover];
                        
                        // 檢查錢夠不夠
                        if (price > DC->player->coin) {
                            debug_log("<UI> Not enough money for robot.\n");
                        } else {
                            // 錢夠 -> 選中該機器人 -> 進入 SELECT 狀態 (準備放置)
                            selected_robot = on_robot_hover; 
                            state = STATE::SELECT; 
                        }
                    }
                }
                break;
            } 
            
            case STATE::SELECT: {
                // 狀態：手上黏著一隻機器人，等待放置
                
                // 左鍵：確認放置位置 -> 進入 PLACE 檢查
                if(left_click) {
                    state = STATE::PLACE;
                }
                // 右鍵：取消選擇 -> 回到 HALT
                if(right_click) {
                    on_robot_hover = -1;
                    selected_robot = -1;
                    state = STATE::HALT;
                    debug_log("<UI> Cancel selection.\n");
                }
                break;
            } 
            
            case STATE::PLACE: {
                // 狀態：執行放置邏輯
                
                // 只處理機器人 (selected_robot != -1)
                if (selected_robot != -1) {
                    int col = (mouse.x - (GRID_START_X - GRID_W/2)) / GRID_W;
                    int row = (mouse.y - (GRID_START_Y - GRID_H/2)) / GRID_H;

                    // 2. 向 Level 拿格子
                    Block* target_block = DC->level->get_block(col, row);

                    // 3. 判斷是否可以放置
                    // 條件：格子存在 (沒超出邊界) 且 格子是空的 (沒有機器人)
                    if (target_block && !target_block->has_robot()) {
                        
                        // 取得格子中心座標
                        Rectangle rect = target_block->get_region();
                        Point p(rect.center_x(), rect.center_y());

                        // 產生機器人
                        Robot* new_robot = Robot::create_robot(static_cast<RobotType>(selected_robot), p);
                        DC->robots.emplace_back(new_robot);
                        
                        // ★★★ 關鍵：告訴這個 Block，你上面有人了！ ★★★
                        target_block->set_robot(new_robot);

                        // 扣錢 & 重置狀態
                        DC->player->coin -= robot_cost[selected_robot];
                        selected_robot = -1;
                        on_robot_hover = -1;
                        state = STATE::HALT;
                        debug_log("<UI> Robot placed at (%d, %d)\n", col, row);
                    } 
                    else {
                        // 放置失敗 (出界 或 該格已有人)
                        debug_log("<UI> Cannot place: Occupied or Out of bounds.\n");
                    }
                }
                else {
                    // 如果莫名其妙進來這裡但沒選機器人，直接重置
                    state = STATE::HALT;
                }

                // ★★★ 新增：放置學生 ★★★
                if (selected_student_type != -1) {
                    // 1. 檢查放置位置 (跟機器人一樣，檢查格子、重疊)
                    // ...
                    
                    // 2. 放置成功
                    // DC->students.emplace_back(Student::create_student(..., mouse));
                    
                    // 3. 回復狀態
                    selected_student_type = -1;
                    state = STATE::HALT;
                }
                break;
            }
        }
    }
}

// 在 UI.cpp 中

// 確保有這些常數定義 (放在 UI.cpp 最上方)
// 如果你的 ROBOT_MENU_X 等變數在 update 已經定義過，這裡不需要重複定義，直接用即可。
// 這裡為了完整性我再列一次，請確保不要重複宣告。
/*
const int ROBOT_MENU_X = 170;
const int ROBOT_MENU_Y = 7;
const int ROBOT_W = 60;
const int ROBOT_H = 60;
const int ROBOT_GAP = 25;
*/

void UI::draw() {
    DataCenter *DC = DataCenter::get_instance();
    FontCenter *FC = FontCenter::get_instance();
    ImageCenter *IC = ImageCenter::get_instance(); // 取得 ImageCenter
    const Point &mouse = DC->mouse;


    if(DC->levels == 2)
    {
        // ★★★ 1. 畫輸送帶背景 (簡單畫個灰色長條) ★★★
        al_draw_filled_rectangle(
            BELT_START_X - 10, BELT_Y - 5, 
            DC->window_width, BELT_Y + BELT_CARD_W + 5, 
            al_map_rgb(50, 50, 50)
        );

        // ★★★ 2. 畫輸送帶上的卡片 ★★★
        for (const auto &card : conveyor_belt) {
            if (card.img) {
                int w = al_get_bitmap_width(card.img);
                int h = al_get_bitmap_height(card.img);
                
                // 畫在 card.x, card.y
                al_draw_scaled_bitmap(card.img, 0, 0, w, h, 
                                    card.x, card.y, BELT_CARD_W, BELT_CARD_W, 0);
                                    
                // 畫個框框美化
                al_draw_rectangle(card.x, card.y, card.x + BELT_CARD_W, card.y + BELT_CARD_W, al_map_rgb(255, 255, 255), 2);
            }
        }


        if ((state == STATE::SELECT || state == STATE::PLACE) && selected_student_type != -1) {
        
            // 1. 定義參數 (需與 update 一致)
            const int GRID_W = 56; 
            const int GRID_H = 80;
            const int GRID_START_X = 50; 
            const int GRID_START_Y = 110;
            const int MAP_COLS = 12;
            const int MAP_ROWS = 5;
            const int COL_LIMIT = 8; // ★ 右邊限制

            // 2. 畫出可放置區域提示 (綠色半透明框)
            // 讓玩家一眼就知道只能放右邊
            if (true) {
                int valid_x = GRID_START_X + COL_LIMIT * GRID_W;
                int valid_y = GRID_START_Y;
                int valid_w = std::abs(MAP_COLS - COL_LIMIT) * GRID_W;
                int valid_h = MAP_ROWS * GRID_H;

                al_draw_filled_rectangle(
                    valid_x, valid_y,
                    valid_x + valid_w, valid_y + valid_h,
                    al_map_rgba(0, 255, 0, 30) // 很淡的綠色
                );
            }

            // 3. 計算「吸附」座標
            // 算出目前滑鼠指在哪一格
            int col = (mouse.x - (GRID_START_X - GRID_W/2)) / GRID_W;
            int row = (mouse.y - (GRID_START_Y - GRID_H/2)) / GRID_H;

            // 判斷合法性
            bool is_valid = true;
            if (true) {
                if (col < COL_LIMIT || col >= MAP_COLS || row < 0 || row >= MAP_ROWS) {
                    is_valid = false;
                }
            }

            // 取得圖片
            char buffer[100];
            sprintf(buffer, "%s/%d.png", StudentSetting::student_imgs_root_path[selected_student_type], 0);
            ALLEGRO_BITMAP *preview = IC->get(buffer);
            
            if (preview) {
                int w = al_get_bitmap_width(preview);
                int h = al_get_bitmap_height(preview);
                float scale = 0.1f; 
                float dw = w * scale;
                float dh = h * scale;

                // ★★★ 關鍵：計算吸附後的繪圖座標 ★★★
                // 如果在範圍內，我們強制把圖畫在「格子中心」
                // 如果在範圍外，我們把圖畫在滑鼠旁邊，或是限制在邊界上
                
                // 1. 限制 col/row 不要超出地圖索引 (Clamp)
                if (col < 0) col = 0;
                if (col >= MAP_COLS) col = MAP_COLS - 1;
                if (row < 0) row = 0;
                if (row >= MAP_ROWS) row = MAP_ROWS - 1;

                // 2. 反算出格子的中心像素座標 (Snap)
                float snap_x = GRID_START_X + col * GRID_W + GRID_W/2;
                float snap_y = GRID_START_Y + row * GRID_H + GRID_H/2;


                // A. 先算出「格子的正中心」在哪裡
                int cell_center_x = GRID_START_X + col * GRID_W + GRID_W / 2;
                int cell_center_y = GRID_START_Y + row * GRID_H + GRID_H / 2;

                // B. 再算出「圖片的左上角」應該畫在哪，才能讓圖片中心對準格子中心
                // 公式： 畫圖座標 = 中心點 - (圖片尺寸 / 2)
                float draw_x = cell_center_x - dw / 2;
                float draw_y = cell_center_y - dh / 2;

                // ==========================================
                // 設定顏色 (合法=白, 非法=紅)
                ALLEGRO_COLOR tint = is_valid ? al_map_rgba_f(1, 1, 1, 0.6) : al_map_rgba_f(1, 0.2, 0.2, 0.6);
                // 3. 畫圖 (使用修正後的 draw_x, draw_y)
                al_draw_tinted_scaled_bitmap(
                    preview, tint, 
                    0, 0, w, h,
                    draw_x, draw_y,  // ★ 用這個新的座標
                    dw, dh, 0
                );

                // 5. 畫一個白框，更明確顯示選中了哪一格
                if (is_valid) {
                    al_draw_rectangle(
                        snap_x - GRID_W/2 + 1, snap_y - GRID_H/2 + 1,
                        snap_x + GRID_W/2 - 1, snap_y + GRID_H/2 - 1,
                        al_map_rgb(255, 255, 255), 2
                    );
                }
            }
        }
    }
  if(DC->levels == 1) {

        // card for typhoon + wind
        ALLEGRO_BITMAP* card = IC->get("./assets/image/card.png");

        float scale_card = 0.55f;

        int card_w = al_get_bitmap_width(card);
        int card_h = al_get_bitmap_height(card);

        // 縮放後尺寸
        float draw_w = card_w * scale_card;
        float draw_h = card_h * scale_card;

        // 左下角
        float x = 65;
        float y = DC->window_height - draw_h + 10;

        al_draw_scaled_bitmap(
            card,
            0, 0,                 // 原圖左上
            card_w, card_h,       // 原圖大小
            x, y,                 // 畫到哪
            draw_w, draw_h,       // 縮放後大小
            0
        );


        // typhoon + wind
        if (icon_typhoon && icon_wind)
        {
            float scale = 0.35f;

            int w1 = al_get_bitmap_width(icon_typhoon);
            int h1 = al_get_bitmap_height(icon_typhoon);
            int w2 = al_get_bitmap_width(icon_wind);
            int h2 = al_get_bitmap_height(icon_wind);

            float base_x = 65;                                // 左下起點（x）
            float base_y = DC->window_height - 70;          // 左下起點（y）
            float gap = 40;                                  // ★ Typhoon 與 Wind 的水平間距

            
            // --- Typhoon 圖示 ---
            al_draw_scaled_bitmap(
                icon_typhoon,
                0, 0, w1, h1,
                base_x,
                base_y,
                w1 * scale,
                h1 * scale,
                0
            );
            // 數字寫在圖示右側
            al_draw_textf(
                FC->courier_new[FontSize::MEDIUM],
                al_map_rgb(255,255,255),
                base_x + w1*scale + 10,
                base_y + 10,
                0,
                "%d",
                DC->player->typhoon_count
            );

            // --- Wind 圖示（水平放置，向右偏移 typhoon 寬度 + 間距） ---
            al_draw_scaled_bitmap(
                icon_wind,
                0, 0, w2, h2,
                base_x + w1 * scale + gap,
                base_y,
                w2 * scale,
                h2 * scale,
                0
            );
            al_draw_textf(
                FC->courier_new[FontSize::MEDIUM],
                al_map_rgb(0,0,0),
                base_x + w1 * scale + gap + w2 * scale + 10,
                base_y + 10,
                0,
                "%d",
                DC->player->wind_count
            );
        }


        // ============================================================
        // 1. 畫分數板 (Score Board)
        // ============================================================
        int table_x = 10; 
        int table_y = 10;

        if (score_table) {
            // 畫背景圖
            float scale = 0.5f; 
            int w = al_get_bitmap_width(score_table);
            int h = al_get_bitmap_height(score_table);
            al_draw_scaled_bitmap(score_table, 0, 0, w, h, table_x, table_y, w*scale, h*scale, 0);
            
            // 畫分數文字
            int text_x = table_x + 65; 
            int text_y = table_y + 15; 
            al_draw_textf(
                FC->courier_new[FontSize::MEDIUM], 
                al_map_rgb(60, 40, 20), 
                text_x, text_y, 
                ALLEGRO_ALIGN_LEFT, 
                "%d", DC->player->coin
            );
        }

        // ============================================================
        // ★★★ 一格格血條：整條單色（綠 / 黃 / 紅） ★★★
        // ============================================================

        int hp = DC->player->HP;
        int max_hp = 10;

        float seg_w = 10;
        float seg_h = 20;
        float blood_gap = 3;
        float bar_x = 15;
        float bar_y = 70;

        // ---- 決定整條血條顏色（只看剩餘血量比例）----
        float percent = (float)hp / max_hp;
        ALLEGRO_COLOR filled_color;

        if (percent > 0.66f)
            filled_color = al_map_rgb(50, 220, 70);       // 綠
        else if (percent > 0.33f)
            filled_color = al_map_rgb(255, 215, 0);       // 黃
        else
            filled_color = al_map_rgb(255, 80, 60);       // 紅


        for (int i = 0; i < max_hp; i++)
        {
            float x1 = bar_x + i * (seg_w + blood_gap);
            float y1 = bar_y;
            float x2 = x1 + seg_w;
            float y2 = y1 + seg_h;

            // 有血 → 整條用同一個顏色
            if (i < hp)
                al_draw_filled_rectangle(x1, y1, x2, y2, filled_color);
            else
                al_draw_filled_rectangle(x1, y1, x2, y2, al_map_rgb(180,180,180)); // 灰色空格

            // 外框
            al_draw_rectangle(x1, y1, x2, y2, al_map_rgb(0,0,0), 2);
        }


        // ============================================================
        // 2. 畫機器人選單 (Robot Menu)
        // ============================================================
        float menu_scale = 0.4f; // 選單圖示縮放比例

        for (int i = 0; i < (int)robot_icons.size(); i++) {
            int coin = DC->player->coin;
            
            // 判斷錢夠不夠，夠就用亮圖 (light)，不夠用暗圖 (dark)
            ALLEGRO_BITMAP* bmp = (coin >= robot_cost[i] ? robot_light_icons[i] : robot_icons[i]);
            if (!bmp) continue;

            float raw_w = al_get_bitmap_width(bmp);
            float raw_h = al_get_bitmap_height(bmp);
            float dw = raw_w * menu_scale;
            float dh = raw_h * menu_scale;

            // 計算每個圖示的位置 (跟 update 裡的邏輯一致)
            // 上方已經定義了 ROBOT_MENU_X 等常數
            float draw_x = ROBOT_MENU_X + i * (ROBOT_W + ROBOT_GAP) + (ROBOT_W - dw) / 2;
            float draw_y = ROBOT_MENU_Y + (ROBOT_H - dh) / 2;

            // 畫圖示
            al_draw_scaled_bitmap(bmp, 0, 0, raw_w, raw_h, draw_x, draw_y, dw, dh, 0);
            
            // 畫價格
            // 價格顏色：錢夠顯示黑色，不夠顯示紅色
            ALLEGRO_COLOR price_color = (coin >= robot_cost[i] ? al_map_rgb(0, 0, 0) : al_map_rgb(255, 0, 0));
            
            al_draw_textf(
                FC->courier_new[FontSize::SMALL], 
                price_color, 
                ROBOT_MENU_X + i * (ROBOT_W + ROBOT_GAP) + ROBOT_W / 2, 
                ROBOT_MENU_Y + ROBOT_H + 3, 
                ALLEGRO_ALIGN_CENTRE, 
                "%d", robot_cost[i]
            );

            // 畫黃色選取框 (Hover 效果)
            if (state == STATE::HOVER && on_robot_hover == i) {
                float bx = ROBOT_MENU_X + i * (ROBOT_W + ROBOT_GAP);
                al_draw_rectangle(
                    bx, ROBOT_MENU_Y, 
                    bx + ROBOT_W, ROBOT_MENU_Y + ROBOT_H, 
                    al_map_rgb(255, 255, 0), // 黃色
                    3 // 線條粗細
                );
            }
        }
    }

    // debug

    // if (true) { // 發布遊戲時改成 false
    
    //     // 使用你上面定義的參數
    //     const int GRID_W = 56; 
    //     const int GRID_H = 80;
    //     const int GRID_START_X = 50; 
    //     const int GRID_START_Y = 110;
    //     const int MAP_COLS = 12;
    //     const int MAP_ROWS = 5;

    //     for (int r = 0; r < MAP_ROWS; r++) {
    //         for (int c = 0; c < MAP_COLS; c++) {
                
    //             // 用公式算出左上角
    //             float x1 = GRID_START_X + c * GRID_W;
    //             float y1 = GRID_START_Y + r * GRID_H;
                
    //             // 算出右下角
    //             float x2 = x1 + GRID_W;
    //             float y2 = y1 + GRID_H;

    //             // 畫紅色的框框 (線條寬度 2)
    //             al_draw_rectangle(x1, y1, x2, y2, al_map_rgb(255, 0, 0), 2);
                
    //             // (選用) 畫中心點
    //             al_draw_filled_circle(x1 + GRID_W/2, y1 + GRID_H/2, 3, al_map_rgb(255, 255, 0));
    //         }
    //     }
    // }

    // ============================================================
    // 4. 狀態處理 (預覽圖與特殊效果)
    // ============================================================
    switch(state) {
        
        case STATE::HALT: {
            break;
        } 
        
        case STATE::HOVER: {
            break;
        }

        case STATE::SELECT:
        case STATE::PLACE: {
            // 預覽機器人 (換成實際圖片) ★★★
            if (selected_robot != -1) {
                // 1. 取得機器人實際圖片 (讀取 0.png)
                char buffer[100];
                sprintf(buffer, "%s/%d.png", 
                    RobotSetting::robot_imgs_root_path[selected_robot], 
                    0 // 使用第 0 張圖當預覽
                );
                
                ALLEGRO_BITMAP *preview_bmp = IC->get(buffer);
                
                // 如果讀不到圖，就退而求其次用商店圖示 (防呆)
                if (!preview_bmp) {
                    preview_bmp = robot_light_icons[selected_robot];
                }

                if (preview_bmp) {
                    // 2. 設定縮放比例 
                    float scale = 0.1f; // 這裡的數字跟 Robot.cpp 一樣
                    
                    int w = al_get_bitmap_width(preview_bmp);
                    int h = al_get_bitmap_height(preview_bmp);
                    float dw = w * scale;
                    float dh = h * scale;

                    // ★★★ 1. 計算「網格吸附」座標 ★★★
					// 算出滑鼠目前在哪一行 (col) 哪一列 (row)
					int col = (mouse.x - (GRID_START_X - GRID_W/2)) / GRID_W;
					int row = (mouse.y - (GRID_START_Y - GRID_H/2)) / GRID_H;

					// 限制範圍 (Clamp)，避免預覽圖跑到格子外面
					if (col < 0) col = 0;
					if (col >= GRID_COLS) col = GRID_COLS - 1;
					if (row < 0) row = 0;
					if (row >= GRID_ROWS) row = GRID_ROWS - 1;

					// 反算出該格子的中心點座標
					int snap_x = GRID_START_X + col * GRID_W;
					int snap_y = GRID_START_Y + row * GRID_H;

					// ★★★ 2. 畫在吸附後的座標上 (snap_x, snap_y) ★★★
					al_draw_tinted_scaled_bitmap(
						preview_bmp,
						al_map_rgba_f(1, 1, 1, 0.5), // 半透明
						0, 0, w, h,
						snap_x - dw/2, snap_y - dh/2, // 置中於格子
						dw, dh,
						0
					);
					
					// (選用) 畫一個白框提示目前選中的格子
					al_draw_rectangle(
						snap_x - GRID_W/2 + 2, snap_y - GRID_H/2 + 2,
						snap_x + GRID_W/2 - 2, snap_y + GRID_H/2 - 2,
						al_map_rgb(255, 255, 255), 2
					);
				}
            }
            break;
        }
    }

    
}

void UI::clear_belt()
{
    // ==========================================
    // ★★★ 新增：重置輸送帶狀態 ★★★
    // ==========================================
    
    // 1. 清空 vector
    conveyor_belt.clear();
    
    // 2. 重置生成計時器 (讓它一開始就馬上生一張，或是設為 0)
    conveyor_spawn_timer = 0; 
    
    // 3. 重置手上選中的學生 (避免重置後手上還黏著怪)
    selected_student_type = -1;
    
    // 4. 重置其他狀態
    state = STATE::HALT;
    on_robot_hover = -1;
    selected_robot = -1;
    on_item = -1;
    
    // ==========================================
}