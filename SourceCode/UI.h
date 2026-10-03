#ifndef UI_H_INCLUDED
#define UI_H_INCLUDED

#include <allegro5/bitmap.h>
#include <vector>
#include <tuple>
#include "./shapes/Point.h"
#include "robots/Robot.h"
#include "students/Student.h" 
#include <deque> // 或者用 vector 也可以

// ★★★ 定義輸送帶卡片 ★★★
struct ConveyorCard {
    StudentType type;    // 這是什麼學生
    float x, y;          // 目前在畫面上的位置
    float target_x;      // 目標位置 (用來做滑動動畫)
    ALLEGRO_BITMAP* img; // 圖片緩存
};

class UI
{
public:
	UI() {}
	void init();
	void update();
	void draw();
	void clear_belt();
private:
	enum class STATE {
		HALT, // -> HOVER
		HOVER, // -> HALT, SELECT
		SELECT, // -> HALT, PLACE
		PLACE // -> HALT
	};
	STATE state;
	ALLEGRO_BITMAP *score_table; // 左上角計分板
	// tower menu bitmap, (top-left x, top-left y), price
	std::vector<std::tuple<ALLEGRO_BITMAP*, Point, int>> tower_items;
	int on_item;
	// robot menu
	std::vector<ALLEGRO_BITMAP*> robot_icons;         // 原本 dark 圖示
	std::vector<ALLEGRO_BITMAP*> robot_light_icons;   // 新增 light 圖示
	std::vector<int> robot_cost;                      // 每個機器人的分數門檻
	int selected_robot = -1;    // -1 表示沒選
	// ★★★ 新增這個：記錄滑鼠現在懸停在選單的哪一個機器人上 ★★★
    int on_robot_hover = -1;
	// typhoon + wind
	ALLEGRO_BITMAP* icon_typhoon;
	ALLEGRO_BITMAP* icon_wind;

	// ★★★ 新增：取消按鈕圖片 ★★★
    ALLEGRO_BITMAP *icon_cancel = nullptr;

	// ★★★ 輸送帶相關變數 ★★★
    std::vector<ConveyorCard> conveyor_belt; 
    int conveyor_spawn_timer;     // 生成倒數
    int selected_student_type = -1; // -1 代表沒選，>=0 代表選中的學生種類
};

#endif
