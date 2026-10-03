#include "Beam.h"
#include "Robot.h"
#include "../Player.h"
#include "../data/DataCenter.h"
#include "../data/ImageCenter.h"
#include "../shapes/Circle.h"
#include "../shapes/Point.h"
#include "../shapes/Rectangle.h"
#include <algorithm>
#include <allegro5/bitmap_draw.h>
#include <allegro5/allegro_primitives.h>
#include <cmath>

// 這邊先保留 target 參數，但我們不會用它（全部直線往前）
Beam::Beam(const Point &p,
           RobotType type,
           const std::string &path,
           double vx, double vy, double ay,
           int dmg,
           double fly_dist)
{
    ImageCenter *IC = ImageCenter::get_instance();

    this->start_x = p.x;
    this->start_y = p.y;
    this->vx = vx;
    this->vy = vy;
    this->ay = ay;
    this->fly_dist = fly_dist;
    this->dmg = dmg;
    this->type = type;
    this->clicked = false;
    this->is_collected = false;
    this->lifetime = 0;
    frame_id = 0;        
    frame_counter = 0; 
    rotation = 0;
    rotation_v = 8;   // 每幀旋轉 8 度，可自行調整
    bitmap = IC->get(path);
    if (type == RobotType::GENERATOR) {
        frame_ids = {0, 1};
        frame_freq = 12;  // 每 12 frame 換一張
    } 
    else if (type == RobotType::LIGHT) {
        frame_ids = {0, 1, 2};
        frame_freq = 30;  // 每 30 frame 換一張
    }
    else if (type == RobotType::ICE) {
        frame_ids = {0, 1, 2, 3};
        frame_freq = 30;  // 每 30 frame 換一張
    }
    else {
        frame_ids = {0};  // 其他 robot 先用單張
    }

    // double r = std::min(al_get_bitmap_width(bitmap),
    //                     al_get_bitmap_height(bitmap)) * 0.4;
    //shape.reset(new Circle{p.x, p.y, r});
    int w = al_get_bitmap_width(bitmap);
    int h = al_get_bitmap_height(bitmap);

    shape.reset(new Rectangle{
        p.x - w/2,
        p.y - h/2,
        p.x + w/2,
        p.y + h/2
    });
}

/**
 * @brief Update the beam position by its velocity and fly_dist by its movement per frame.
 */
void Beam::update() {
    //if (fly_dist == 0) return;

    DataCenter *DC = DataCenter::get_instance();
    double dt = 1.0 / DC->FPS;

    // ====================================================
    // ★★★ 新增：被收集時的飛行邏輯 (飛向左上角) ★★★
    // ====================================================
    if (is_collected) {
        // 1. 設定目標點 (左上角分數板星星的位置，大約 40, 40)
        double target_x = 40;
        double target_y = 40;

        // 2. 計算方向向量
        double dx = target_x - shape->center_x();
        double dy = target_y - shape->center_y();
        double dist = std::sqrt(dx*dx + dy*dy);

        // 3. 判斷是否到達 (距離小於 10)
        if (dist < 10) {
            // ★ 到達了！加錢並消失 ★
            DC->player->coin += 5; // 加 5 元
            fly_dist = 0;           // 標記刪除
            return;
        }

        // 4. 設定飛行速度 (例如每秒 800 像素，飛快一點)
        double speed = 800.0;
        vx = (dx / dist) * speed;
        vy = (dy / dist) * speed;
        ay = 0; // 關閉重力，直線飛行
        shape->update_center_x(shape->center_x() + vx * dt);
        shape->update_center_y(shape->center_y() + vy * dt);
        return;
    }
    // ====================================================

    if (type == RobotType::GENERATOR) {
        lifetime++;

        if (lifetime > 240 && clicked == false) {
            fly_dist = 0;  // 讓 OperationCenter 自己刪掉它
            return;        // ⚠ 很重要！直接結束，不畫也不動
        }
    }

    if (!frame_ids.empty()) {
        if (frame_counter > 0) {
            frame_counter--;
        } else {
            frame_id = (frame_id + 1) % frame_ids.size();
            frame_counter = frame_freq;
        }
    }

    switch (type) {

        case RobotType::GENERATOR:
            if (!is_collected && shape->center_y() >= start_y + 30) {
                vx = vy = ay = 0;
                frame_id = 1;
                return;
            }
            break;
        default:
            break;
    }
        
    // 拋物線：每幀速度更新
    vy += ay * dt;

    double dx = vx / DC->FPS;
    double dy = vy / DC->FPS;

    double movement = Point::dist(Point{dx, dy}, Point{0, 0});
    if (fly_dist > movement) {
        shape->update_center_x(shape->center_x() + dx);
        shape->update_center_y(shape->center_y() + dy);
        fly_dist -= movement;
    } else {
        // 剩下不到一格距離，就只移動剩餘的 fly_dist
        if (movement > 0) {
            double ratio = fly_dist / movement;
            shape->update_center_x(shape->center_x() + dx * ratio);
            shape->update_center_y(shape->center_y() + dy * ratio);
        }
        fly_dist = 0;
    }



    rotation += rotation_v;
    if (rotation >= 360) rotation -= 360;

}

void Beam::draw() {
    // al_draw_bitmap(
    //     bitmap,
    //     shape->center_x() - al_get_bitmap_width(bitmap) / 2,
    //     shape->center_y() - al_get_bitmap_height(bitmap) / 2,
    //     0
    // );

    ImageCenter *IC = ImageCenter::get_instance();
    double cx = shape->center_x();
    double cy = shape->center_y();


    ALLEGRO_BITMAP* bmp;

    if (type == RobotType::GENERATOR) {
        char path[100];
        sprintf(path,
            "./assets/image/robot/Generator_Beam%d.png",
            frame_ids[frame_id]
        );
        bmp = IC->get(path);
    }
    else if (type == RobotType::LIGHT) {
        char path[100];
        sprintf(path,
            "./assets/image/robot/Light_Beam%d.png",
            frame_ids[frame_id]
        );
        bmp = IC->get(path);
    } 
    else if (type == RobotType::ICE) {
        char path[100];
        sprintf(path,
            "./assets/image/robot/Ice_Beam%d.png",
            frame_ids[frame_id]
        );
        bmp = IC->get(path);
    }
    else {
        bmp = bitmap;  // 其他 robot 使用原本單一張
    }

    if (!bmp) return;

    int w = al_get_bitmap_width(bmp);
    int h = al_get_bitmap_height(bmp);

    // ------ 動態大小調整 (你可以自己改規則) ------
    double scale = 0.3;        // 原始大小
    // EX1：讓子彈隨著飛行距離變小（遠方變小）
    // scale = std::max(0.2, fly_dist / 1200.0);

    // EX2：隨機大小
    // scale = rand_range(0.5, 1.2);

    // EX3：不同 Robot 子類別給不同 scale
    // scale = isGenerator ? 0.8 : 1.0;

    // ----------------------------------------------

    if (type == RobotType::ADV1) {

        float cx0 = al_get_bitmap_width(bmp) / 2.0f;
        float cy0 = al_get_bitmap_height(bmp) / 2.0f;

        float scale = 0.3;

        al_draw_scaled_rotated_bitmap(
            bmp,
            cx0, cy0,     // 圖片中心
            shape->center_x(),
            shape->center_y(),
            scale, scale,   // 縮放（ADV1 子彈也要縮小）
            rotation * std::acos(-1.0) / 180.0,  // 旋轉角度
            0
        );

        // ★★★ IMPORTANT：不要讓後面 hitbox 覆蓋 rotated bitmap 導致視覺不見
        return;
    }
    else {
        al_draw_scaled_bitmap(
            bmp,
            0, 0, w, h,                  // source
            cx - (w * scale) / 2,
            cy - (h * scale) / 2,        // destination
            w * scale,
            h * scale,
            0
        );

        Rectangle* r = dynamic_cast<Rectangle*>(shape.get());
        if (r) {
            double hw = (w * scale) / 2;  // half width
            double hh = (h * scale) / 2;  // half height

            r->x1 = cx - hw;
            r->y1 = cy - hh;
            r->x2 = cx + hw;
            r->y2 = cy + hh;

            // // 🔥 debug 繪製 Rectangle hitbox
            // al_draw_rectangle(
            //     r->x1, r->y1,
            //     r->x2, r->y2,
            //     al_map_rgb(255, 0, 0),
            //     2
            // );
        }
    }

    // Circle* c = dynamic_cast<Circle*>(shape.get());
    // if (c) {
    //     double r = std::min(w, h) * 0.5 * scale;

    //     c->r = r;
    //     c->x = cx;
    //     c->y = cy;

    //     al_draw_circle(
    //         c->x,
    //         c->y,
    //         c->r,
    //         al_map_rgb(255, 0, 0),
    //         2
    //     );
    // }
    
}
