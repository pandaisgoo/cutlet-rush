#include "Level.h"
#include <string>
#include "Utils.h"
#include "students/StudentNorm.h"
#include "students/StudentBicycle.h"
#include "students/StudentDrill.h"
#include "students/StudentJump.h"
#include "students/StudentBooo.h"
#include "special/SpecialTyphoon.h"
#include "special/SpecialTyphoon.h"
#include "special/SpecialWind.h"
#include "robots/RobotShield.h"
#include "robots/RobotLight.h"
#include "robots/RobotIce.h"
#include "robots/RobotAdv1.h"
#include "robots/RobotAdv2.h"
#include "Star.h"
#include "data/DataCenter.h"
#include "data/ImageCenter.h"
#include <allegro5/allegro_primitives.h>
#include "shapes/Point.h"
#include "shapes/Rectangle.h"
#include <array>
#include <algorithm>

using namespace std;

// fixed settings
namespace LevelSetting {
	constexpr char level_path_format[] = "./assets/level/LEVEL%d.txt";
	//! @brief Grid size for each level.
	constexpr array<int, 4> grid_size = {
		40, 40, 40, 40
	};
	constexpr int monster_spawn_rate = 90;
	constexpr int student_spawn_rate = 90;
};

// 格子開始的位置
const int GRID_START_X = 175;
const int GRID_START_Y = 140;
const int GRID_W = 56;
const int GRID_H = 80;

void
Level::init() {
	level = -1;
	debug_log("Level::init called. Creating grid...\n");
	grid_w = -1;
	grid_h = -1;
	student_spawn_counter = 0;
	special_spawn_counter = 300; // ★ 每 120 frame（5 秒）生成一個 special

	// ★★★ 初始化格子 ★★★
    grid.clear();
    grid.resize(GRID_ROWS); // 設定有幾列

    for (int r = 0; r < GRID_ROWS; r++) {
        grid[r].resize(GRID_COLS); // 設定這列有幾行
        for (int c = 0; c < GRID_COLS; c++) {
            // 計算座標
            int x = GRID_START_X + c * GRID_W;
            int y = GRID_START_Y + r * GRID_H;
            
            // 建立 Block
            grid[r][c] = new Block(x - GRID_W/2, y - GRID_H/2, GRID_W, GRID_H);

            // 設定西洋棋盤顏色 (深綠 / 淺綠)
            if ((r + c) % 2 == 0) {
                grid[r][c]->set_color(al_map_rgb(34, 139, 34)); // ForestGreen
            } else {
                grid[r][c]->set_color(al_map_rgb(50, 205, 50)); // LimeGreen
            }
        }
    }
}	

/**
 * @brief Loads level data from input file. The input file is required to follow the format.
 * @param lvl level index. The path format is a fixed setting in code.
 * @details The content of the input file should be formatted as follows:
 *          * Total number of monsters.
 *          * Number of each different number of monsters. The order and number follows the definition of MonsterType.
 *          * Indefinite number of Point (x, y), represented in grid format.
 * @see level_path_format
 * @see MonsterType
 */
void
Level::load_level(int lvl) {
	DataCenter *DC = DataCenter::get_instance();

	char buffer[50];
	sprintf(buffer, LevelSetting::level_path_format, lvl);
	FILE *f = fopen(buffer, "r");
	GAME_ASSERT(f != nullptr, "cannot find level.");
	level = lvl;
	grid_w = DC->game_field_length / LevelSetting::grid_size[lvl];
	grid_h = DC->game_field_length / LevelSetting::grid_size[lvl];
	road_path.clear();

    if(lvl == 2){
        // ★ 先清乾淨（避免重進關卡殘留）
        reset();
        DC->robots.clear();   // 指標清單（真正 delete 在 reset）
        spawn_fixed_robots_level2();
    }
}

/**
 * @brief Updates monster_spawn_counter and create monster if needed.
*/
void
Level::update() {

    DataCenter* DC = DataCenter::get_instance();

    // =============================================================
    // ★★★ 新增：如果是 Level 2 (輸送帶模式)，停止自動生怪 ★★★
    // =============================================================
    // 因為 Level 2 是由玩家手動放置學生，所以不需要讀取 level.txt 的生怪腳本
    if (DC->levels == 2) {
        return; 
    }
    // =============================================================
    static int star_spawn_counter = 60;  // 每 180frame = 3秒
    star_spawn_counter--;

    if (star_spawn_counter <= 0) {
        spawn_star();
        star_spawn_counter = 60 + rand()%120; // 
    }

	if (special_spawn_counter > 0) {
        special_spawn_counter--;
    } else {
        spawn_special();             // ★ 呼叫 special 生成
        special_spawn_counter = 300; // ★ 重設 5 秒間隔
    }

	if (student_spawn_counter > 0) {
		student_spawn_counter--;
	} else {
		spawn_student();  // ← 生成一個 NORM 學生
		student_spawn_counter = 120;  // 每 30 frame 生成一個學生
	}


    // ★★★★★ 清理過期的 Star ★★★★★
    auto& stars = DC->stars;

    stars.erase(
        remove_if(stars.begin(), stars.end(),
            [&](Star* s){
                if (s->collected) {    // collected = true → 表示停留時間到 或 玩家收集
                    delete s;
                    return true;
                }
                return false;
            }),
        stars.end()
    );

    

    // ★★★★★ 在最後清掉死掉的學生 ★★★★★
    auto& S = DC->students;
	auto&specials = DC->specials;

    S.erase(
        remove_if(S.begin(), S.end(),
            [&](Student* stu){
                if (stu->HP <= 0) {
                    delete stu;         // ★ 釋放記憶體
                    return true;        // 從 vector 移除
                }
                return false;
            }),
        S.end()
    );
	specials.erase(
		remove_if(specials.begin(), specials.end(),
			[&](Special* sp){
				if (sp->dying == false) return false;     // 還沒開始死亡動畫
				if (sp->death_timer > 0) return false;    // 死亡動畫還在跑

				if (sp->HP <= 0) {
					delete sp;
					return true;
				}
				return false;
			}),
		specials.end()
	);
}

void Level::reset() {
    DataCenter* DC = DataCenter::get_instance();    
    // 1️⃣ 刪掉所有 robot 物件
    for (Robot* r : DC->robots) {
        delete r;
    }
    DC->robots.clear();

    // 2️⃣ 清掉每個 Block 的 robot 指標
    // 遍歷二維陣列
    for (int r = 0; r < GRID_ROWS; r++) {
        for (int c = 0; c < GRID_COLS; c++) {
            if (grid[r][c]) {
                // 呼叫 Block 的 clear_robot()，將指標歸零
                grid[r][c]->clear_robot();
            }
        }
    }
}

void
Level::draw() {
	// 1. 先畫格子 (當作地板)
    for (int r = 0; r < GRID_ROWS; r++) {
        for (int c = 0; c < GRID_COLS; c++) {
            if(grid[r][c]) grid[r][c]->draw();
        }
    }
	if(level == -1) return;
}


Block* Level::get_block(int col, int row) {
    if (!is_in_grid(col, row)) return nullptr;
    return grid[row][col];
}


bool Level::is_onroad(const Rectangle &region) {
    if (level == 2) {
        return true; // ★ Level 2 所有學生都算可攻擊
    }
    for(const Point &grid : road_path) {
        if(grid_to_region(grid).overlap(region))
            return true;
    }
    return false;
}

Rectangle
Level::grid_to_region(const Point &grid) const {
	int x1 = grid.x * LevelSetting::grid_size[level];
	int y1 = grid.y * LevelSetting::grid_size[level];
	int x2 = x1 + LevelSetting::grid_size[level];
	int y2 = y1 + LevelSetting::grid_size[level];
	return Rectangle{x1, y1, x2, y2};
}

// void Level::spawn_student() {
//     DataCenter* DC = DataCenter::get_instance();

//     // ---------- 1. 隨機決定學生種類 ----------
//     StudentType type;
//     int r = rand() % 5;  // 目前：0=Norm, 1=Bicycle, 2=Drill, 3=Jump, 4=Booo

//     if (r == 0)
//         type = StudentType::NORM;
//     else if (r == 1)
//         type = StudentType::BICYCLE;
// 	else if (r == 2)
// 	    type = StudentType::DRILL;
// 	else if (r == 3)
// 	    type = StudentType::JUMP;
// 	else if (r == 4)
// 	    type = StudentType::BOOO;
// 	else
//     	type = StudentType::NORM; // 防止未初始化

//     // ---------- 2. 建立學生物件（給空 path，不再使用 grid） ----------
//     std::vector<Point> fake;
//     fake.push_back(Point{0, 0});

//     Student* stu = Student::create_student(type, fake);

//     // ---------- 3. 設定生成位置：畫面右邊 + 隨機高度 ----------
//     float margin = 100;
//     float start_x = DC->window_width + 50;
//     float start_y = margin + (rand() % (int)(DC->window_height - 2 * margin));

//     stu->shape->update_center_x(start_x);
//     stu->shape->update_center_y(start_y);

//     // ---------- 4. 加入學生列表 ----------
//     DC->students.push_back(stu);
// }


void Level::spawn_student() {
    DataCenter* DC = DataCenter::get_instance();

    // ---------- 1. 隨機決定學生種類 ----------
    StudentType type;
    int r = rand() % 5;  // 目前：0=Norm, 1=Bicycle, 2=Drill, 3=Jump, 4=Booo

    if (r == 0)      type = StudentType::NORM;
    else if (r == 1) type = StudentType::BICYCLE;
    else if (r == 2) type = StudentType::JUMP;
    else if (r == 3) type = StudentType::DRILL;
    else             type = StudentType::JUMP;

    // ---------- 2. 建立學生物件（path 已無用） ----------
    std::vector<Point> fake;
    fake.push_back(Point{0, 0});
    Student* stu = Student::create_student(type, fake);

    // ---------- 3. 固定可選的 Y 軸位置 ----------
    static const int allowedY[] = { 140, 220, 300, 380, 460 };
    int index = rand() % 5;
    float start_y = allowedY[index];

    // ---------- 4. x 軸固定從畫面右方 900 ----------
    float start_x = 900;  // 或 = DC->window_width + 50;

    // ---------- 5. 設定座標 ----------
    stu->shape->update_center_x(start_x);
    stu->shape->update_center_y(start_y);

    // ---------- 6. 加入學生列表 ----------
    DC->students.push_back(stu);
}

void Level::spawn_special() {
    DataCenter* DC = DataCenter::get_instance();

    // 1) 隨機 Y 軸
    static const int allowedY[] = { 140, 220, 300, 380, 460 };
    int index = rand() % 5;

    float start_x = 900;
    float start_y = allowedY[index];

    // 2) Special 需要 path，但我們不用路線，所以給假的
    std::vector<Point> fake_path;
    fake_path.push_back(Point{0, 0});

    // 3) 隨機 Typhoon 或 Wind
    Special* sp;
    if (rand() % 2 == 0)
        sp = new SpecialTyphoon(fake_path);
    else
        sp = new SpecialWind(fake_path);

    // 4) 強制設定生成位置
    sp->shape->update_center_x(start_x);
    sp->shape->update_center_y(start_y);

    // 5) 加入 specials
    DC->specials.push_back(sp);
}

void Level::spawn_star() {
    DataCenter* DC = DataCenter::get_instance();
    ImageCenter* IC = ImageCenter::get_instance();

    ALLEGRO_BITMAP* bmp = IC->get("./assets/image/robot/Generator_Beam1.png"); // 你星星圖片的路徑

    float x = 200 + rand() % (DC->window_width - 300);
    float y = -50;  // 天上掉下來

    Star* star = new Star(x, y, bmp);
    DC->stars.push_back(star);
}

void Level::spawn_fixed_robots_level2()
{
    DataCenter* DC = DataCenter::get_instance();

    struct RobotInfo { RobotType type; int col; int row; };

    std::vector<RobotInfo> robots = {
        {RobotType::ADV1, 3, 1},
        {RobotType::ADV1, 0, 0},
        //{RobotType::ADV1, 0, 1},
        {RobotType::ADV1, 0, 2},
        //{RobotType::ADV1, 0, 3},
        {RobotType::ADV1, 0, 4},
        {RobotType::ICE,  1, 1},
        //{RobotType::ICE,  1, 4},
        {RobotType::ICE,  2, 3},
        //{RobotType::ICE,  2, 0},
        {RobotType::ICE,  2, 1},
        //{RobotType::ICE,  3, 0},
        {RobotType::ICE,  3, 2},
        {RobotType::LIGHT,  2, 2},
        //{RobotType::LIGHT,  1, 2},
        {RobotType::LIGHT,  1, 0},
        {RobotType::LIGHT,  1, 3},
        {RobotType::SHIELD,  4, 0},
        {RobotType::SHIELD,  4, 1},
        {RobotType::SHIELD,  4, 2},
        {RobotType::SHIELD,  4, 3},
        {RobotType::SHIELD,  4, 4},
    };

    for (const auto& info : robots) {
        Block* blk = get_block(info.col, info.row);
        if (!blk) continue;

        // ✅ 用你棋盤格系統的中心點（和 init 建格一致）
        float cx = GRID_START_X + info.col * GRID_W;
        float cy = GRID_START_Y + info.row * GRID_H;

        Robot* robot = Robot::create_robot(info.type, Point(cx, cy));

        DC->robots.push_back(robot);
        blk->set_robot(robot);
    }
}

Point Level::world_to_block(double x, double y) const
{
    int col = static_cast<int>((x - GRID_START_X + GRID_W / 2) / GRID_W);
    int row = static_cast<int>((y - GRID_START_Y + GRID_H / 2) / GRID_H);

    if (!is_in_grid(col, row)) {
        return Point(-1, -1);
    }
    return Point(col, row);
}

Point Level::block_center(int col, int row) const
{
    return Point(
        GRID_START_X + col * GRID_W,
        GRID_START_Y + row * GRID_H
    );
}
