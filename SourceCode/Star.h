#ifndef STAR_H_INCLUDED
#define STAR_H_INCLUDED

#include "shapes/Rectangle.h"
#include "shapes/Point.h"
#include <allegro5/allegro.h>
#include "data/DataCenter.h"

class Star {
public:
    float x, y;
    float vy = 1.5;             // 掉落速度
    ALLEGRO_BITMAP* bmp;

    Rectangle shape;

    bool collected = false;     // 是否被玩家點擊
    bool stopped = false;       // 是否已抵達底部並停下
    int stay_timer = 0;         // 停留時間（frame）

    Star(float sx, float sy, ALLEGRO_BITMAP* img)
        : x(sx), y(sy), bmp(img),
          shape(sx-15, sy-15, sx+15, sy+15)
    {}

    void update() {

        DataCenter* DC = DataCenter::get_instance();
        float bottom_limit = DC->window_height - 20;

        // ⭐ 若已抵達底部 → 停住並計時
        if (stopped) {
            if (stay_timer > 0) {
                stay_timer--;
            } else {
                // ⭐ 停滿 1 秒 → 標記為要移除
                collected = true;   // 用 collected 當「消失」條件
            }
            return;
        }

        // ⭐ 正常掉落
        y += vy;

        // ⭐ 更新 hitbox
        shape.update_center_y(y);

        // ⭐ 抵達底部 → 停住
        if (y >= bottom_limit) {
            y = bottom_limit;
            shape.update_center_y(y);

            stopped = true;
            stay_timer = 60;   // 停留1秒（60 frame）
        }
    }

    void draw() {
        if (collected) return;  // ⭐ 已到期或被收集 → 不畫

        if (!bmp) return;
        al_draw_scaled_bitmap(
            bmp, 0,0,
            al_get_bitmap_width(bmp),
            al_get_bitmap_height(bmp),
            x-20, y-20, 40, 40,
            0
        );
    }
};

#endif
