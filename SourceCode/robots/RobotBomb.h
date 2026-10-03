#ifndef ROBOTBOMB_H_INCLUDED
#define ROBOTBOMB_H_INCLUDED

#include "Robot.h"
#include "../shapes/Point.h"

class RobotBomb : public Robot
{
public:
    RobotBomb(const Point &p) : Robot{p, RobotType::BOMB} 
    {
        HP = 200;

        // 超大值，讓炸彈不自己切換圖片
        bitmap_img_ids.clear();
        bitmap_img_ids.push_back({0, 1});
        bitmap_switch_freq = 999999999; 
        bitmap_img_id = 0; // 一開始一定是 0.png

        // 設定倒數 3 秒 (60 FPS * 3 = 180)
        explode_timer = 180;
        exploding = false;
    }

    Beam *create_beam() override {
        return nullptr;
    }

    int get_explosion_damage() {
        return 10000;
    }

    bool exploding;      // class 成員
    int explode_timer;   // class 成員
};

#endif
