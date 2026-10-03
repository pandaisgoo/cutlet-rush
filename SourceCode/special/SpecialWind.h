#ifndef SPECIALWIND_H_INCLUDED
#define SPECIALWIND_H_INCLUDED

#include "Special.h"

// fixed settings: StudentTyphoon attributes
class SpecialWind : public Special
{
public:

    SpecialWind(const std::vector<Point> &path)
        : Special{path, SpecialType::WIND, 2} 
    {
        HP = 10;        // 學生血量
        v = 80;
        old_v = v;
        dying = false;
        death_timer = 0;
        death_frame_index = 2; 

        // 0.png：正常站立
        // 1.png：舉手
        // 2.png：你剛剛截圖的那個颱風 icon（死亡用）
        bitmap_img_ids.emplace_back(std::vector<int>({0, 1}));
        bitmap_switch_freq = 20;

        // ★ 不要再在這裡用 if (dying && ...) 了，dying 一開始是 false
        //   所以不用再 loop 找，直接用 death_frame_index = 2 最乾淨
    }
};

#endif
