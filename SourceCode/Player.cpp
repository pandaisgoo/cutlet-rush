#include "Player.h"
#include <iostream>
#include "data/DataCenter.h"
#include "students/Student.h"
#include "special/Special.h"
#include <allegro5/allegro_primitives.h>
#include <fstream>  // ★ 必須引入這個來做檔案讀寫
#include "shapes/Circle.h"

// fixed settings
namespace PlayerSetting {
	constexpr int init_HP = 10;
	constexpr int init_coin = 0;
	constexpr int coin_freq = 60;
	constexpr int coin_increase = 5;
};

Player::Player() : HP(PlayerSetting::init_HP), coin(PlayerSetting::init_coin) {
	this->coin_freq = PlayerSetting::coin_freq;
	this->coin_increase = PlayerSetting::coin_increase;
	coin_counter = PlayerSetting::coin_freq;
    current_kills = 0; // 初始化殺敵數
    // ★ 建構時就載入最高分
    load_high_score(); 
}

// 取得 shape 的左邊界（Rectangle / Circle 都支援）
static float get_left_bound(Shape* shape) {

    if (!shape) return 99999; // 保險

    // Rectangle
    if (auto* r = dynamic_cast<Rectangle*>(shape)) {
        return r->x1;
    }

    // Circle
    if (auto* c = dynamic_cast<Circle*>(shape)) {
        return c->x - c->r;
    }

    // fallback：至少不會亂扣
    return shape->center_x();
}
void Player::update() {
    DataCenter* DC = DataCenter::get_instance();

    // 左側扣血線
    const float LEFT_KILL_ZONE = 10.0f;

    // ===============================
    // ★ 1. 學生 Student 扣血判定
    // ===============================
    for (Student* s : DC->students) {

        float left_edge = get_left_bound(s->shape.get());   // <-- 正確左邊界

        if (left_edge <= LEFT_KILL_ZONE) {

            if (!s->is_counted_dead) {
                HP -= 1;
                s->is_counted_dead = true;  // 不再扣第二次
                s->HP = 0;                  // 交由死亡系統處理
                
                std::cout << "[HP-1] Student reached goal.\n";
            }
        }
    }

    // ===============================
    // ★ 2. Special（Typhoon / Wind）扣血判定
    // ===============================
    for (Special* sp : DC->specials) {

        float left_edge = get_left_bound(sp->shape.get());  // <-- 正確左邊界

        if (left_edge <= LEFT_KILL_ZONE -100) {

            if (!sp->is_counted_dead) {
                HP -= 1;
                sp->is_counted_dead = true;
                sp->HP = 0;   // 觸發死亡動畫
                
                std::cout << "[HP-1] Special reached goal.\n";
            }
        }
    }

    // ===============================
    // ★ 3. HP 不可為負
    // ===============================
    if (HP < 0) HP = 0;
}

// 如果還有其他變數需要重置（例如分數），也可以寫在這裡
void Player::init() {
    HP = 10;       // 設定你的滿血數值 3
    coin = 100;    // 設定初始金錢 (例如 0)
	typhoon_count = 0;
	wind_count = 0;
    current_kills = 0; // ★ 重置殺敵數
}

// ★★★ 讀取多筆紀錄 ★★★
void Player::load_high_score() {
    high_score_records.clear();
    std::ifstream in("highscore.txt");
    if (in.is_open()) {
        ScoreRecord rec;
        // 格式：分數 殺敵數 輸贏(0/1)
        while (in >> rec.score >> rec.kills >> rec.is_win) {
            high_score_records.push_back(rec);
        }
        in.close();
    }
}

// ★★★ 儲存多筆紀錄 ★★★
void Player::save_high_score() {
    std::ofstream out("highscore.txt");
    if (out.is_open()) {
        for (const auto& rec : high_score_records) {
            out << rec.score << " " << rec.kills << " " << rec.is_win << std::endl;
        }
        out.close();
    }
}

// ★★★ 新增紀錄並排序 ★★★
void Player::add_record(int score, int kills, bool is_win) {
    // 1. 加入新紀錄
    high_score_records.push_back({score, kills, is_win});
    
    // 2. 排序 (分數高的在前面)
    std::sort(high_score_records.begin(), high_score_records.end(), 
        [](const ScoreRecord& a, const ScoreRecord& b) {
            return a.score > b.score;
        }
    );

    // 3. 只保留前 6 名
    if (high_score_records.size() > 6) {
        high_score_records.resize(6);
    }

    // 4. 存檔
    save_high_score();
}