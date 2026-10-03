#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED

#include <string>
#include <vector>
#include <algorithm>

// ★★★ 1. 定義單筆紀錄的結構 ★★★
struct ScoreRecord {
    int score;
    int kills;
    bool is_win; // true = Win, false = Lose

    // overloading > 運算子，方便排序 (分數高的排前面)
    bool operator>(const ScoreRecord& other) const {
        return score > other.score;
    }
};

class Player
{
public:
	Player();
	void update();
	void init();
	int HP;
	int coin; //星星錢
	int typhoon_count;
	int wind_count;

	int high_score; // ★ 新增：最高分
	// ★★★  新增：當前遊戲的殺敵數 ★★★
    int current_kills; 
    // ★★★  改用 vector 儲存前 6 名紀錄 ★★★
    std::vector<ScoreRecord> high_score_records;

    // ★ 新增：讀取與存檔函式
    void load_high_score();
    void save_high_score();
	// ★★★ 新增：加入新紀錄的函式 ★★★
    void add_record(int score, int kills, bool is_win);
private:
	int coin_freq;
	int coin_increase;
	int coin_counter;
};

#endif
