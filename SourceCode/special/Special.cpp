#include "Special.h"
#include "SpecialTyphoon.h"
#include "SpecialWind.h"

#include "../data/DataCenter.h"
#include "../data/ImageCenter.h"
#include "../Level.h"
#include "../Utils.h"
#include "../shapes/Point.h"
#include "../shapes/Rectangle.h"

#include <allegro5/allegro_primitives.h>
#include <cmath>
#include <string>

using namespace std;

namespace SpecialSetting {
    static constexpr char special_imgs_root_path[static_cast<int>(SpecialType::SPECIAL_MAX)][60] = {
        "./assets/image/special/Typhoon",
        "./assets/image/special/Wind"
    };
}

//----------------------------------------
// Factory
//----------------------------------------
Special* Special::create_special(SpecialType type, const vector<Point>& path) {
    switch(type) {
        case SpecialType::TYPHOON:    return new SpecialTyphoon(path);
        case SpecialType::WIND:    return new SpecialWind(path);
        case SpecialType::SPECIAL_MAX: break;
    }
    GAME_ASSERT(false, "Special type error.");
    return nullptr;
}

//----------------------------------------
// Constructor
//----------------------------------------
Special::Special(const vector<Point> &path, SpecialType type, int dmg)
    : type(type)
{
    DataCenter *DC = DataCenter::get_instance();
    shape.reset(new Rectangle{0,0,0,0});
    bitmap_img_id = 0;
    bitmap_switch_counter = 0;
    this->dmg = dmg;
    is_counted_dead = false;

    for (const auto& p : path)
        this->path.push(p);

    if (!this->path.empty()) {
        Point grid = this->path.front();
        Rectangle region = DC->level->grid_to_region(grid);

        shape.reset(new Rectangle{
            region.center_x(),
            region.center_y(),
            region.center_x(),
            region.center_y()
        });

        this->path.pop();
    }
}
void Special::update() {
    DataCenter *DC = DataCenter::get_instance();
    ImageCenter *IC = ImageCenter::get_instance();
    // -------------------------------------------------------
    // ★★★ 若正在飛向左下角（收集動畫），優先處理動畫 ★★★
    // -------------------------------------------------------
    if (flying_to_ui) {

        float t = 1.0f - (float)fly_timer / 20.0f;  // t = 0 → 1

        // 插值位置（線性飛行）
        fly_x = start_x * (1 - t) + ui_target_x * t;
        fly_y = start_y * (1 - t) + ui_target_y * t;

        shape->update_center_x(fly_x);
        shape->update_center_y(fly_y);

        fly_timer--;

        if (fly_timer <= 0) {
            HP = 0;              // 飛行結束 → 特效消失
            flying_to_ui = false;
        }

        return; // ★ 不執行下面的走路/死亡動畫
    }


    // ------------------------------------------------
    // ★ Typhoon Special 死亡動畫邏輯
    // ------------------------------------------------
    if (type == SpecialType::TYPHOON) {

        SpecialTyphoon* t = static_cast<SpecialTyphoon*>(this);

        // (1) 第一次死亡：切到 2.png
        if (!t->dying && HP <= 0) {
            t->dying = true;
            t->death_timer = 600;

            v = 0;   // 停止移動
            old_v = 0;

            bitmap_img_id = t->death_frame_index;  // = 2
            bitmap_switch_counter = 999999;        // 不再換圖

            return;
        }

        // (2) 死亡動畫期間
        if (t->dying) {

            if (t->death_timer > 0) {
                t->death_timer--;

                // 更新 hitbox 使用 2.png
                char buffer[100];
                sprintf(buffer, "%s/%d.png",
                    SpecialSetting::special_imgs_root_path[(int)type],
                    t->death_frame_index   // 指定 2.png
                );

                ALLEGRO_BITMAP* bmp = IC->get(buffer);

                double cx = shape->center_x();
                double cy = shape->center_y();

                int w = al_get_bitmap_width(bmp);
                int h = al_get_bitmap_height(bmp);
                float scale = 0.1f;

                shape.reset(new Rectangle{
                    cx - (w * scale) / 3,
                    cy - (h * scale) / 2,
                    cx + (w * scale) / 3,
                    cy + (h * scale) / 2
                });

                return;
            }
            else {
                // (3) 死亡動畫播放完 → 交 OperationCenter 刪除
                // HP 保持 <= 0
                return;
            }
        }
    }

    // ------------------------------------------------
    // ★ Wind Special 死亡動畫邏輯
    // ------------------------------------------------
    if (type == SpecialType::WIND) {

        SpecialWind* w = static_cast<SpecialWind*>(this);

        // (1) 第一次死亡：切到 2.png
        if (!w->dying && HP <= 0) {
            w->dying = true;
            w->death_timer = 600;

            v = 0;   // 停止移動
            old_v = 0;

            bitmap_img_id = w->death_frame_index;  // = 2
            bitmap_switch_counter = 999999;        // 不再換圖

            return;
        }

        // (2) 死亡動畫期間
        if (w->dying) {

            if (w->death_timer > 0) {
                w->death_timer--;

                // 更新 hitbox 使用 2.png
                char buffer[100];
                sprintf(buffer, "%s/%d.png",
                    SpecialSetting::special_imgs_root_path[(int)type],
                    w->death_frame_index   // 指定 2.png
                );

                ALLEGRO_BITMAP* bmp = IC->get(buffer);

                double cx = shape->center_x();
                double cy = shape->center_y();

                int w = al_get_bitmap_width(bmp);
                int h = al_get_bitmap_height(bmp);
                float scale = 0.1f;

                shape.reset(new Rectangle{
                    cx - (w * scale) / 3,
                    cy - (h * scale) / 2,
                    cx + (w * scale) / 3,
                    cy + (h * scale) / 2
                });

                return;
            }
            else {
                // (3) 死亡動畫播放完 → 交 OperationCenter 刪除
                // HP 保持 <= 0
                return;
            }
        }
    }

    // ------------------------------------------------
    // ★ 以下是一般 Special 邏輯（保持走路動畫 0、1）
    // ------------------------------------------------

    if (bitmap_switch_counter > 0)
        bitmap_switch_counter--;
    else {
        bitmap_img_id = (bitmap_img_id + 1) % bitmap_img_ids[0].size();  // 只會是 0/1
        bitmap_switch_counter = bitmap_switch_freq;
    }

    double dx = -v / DC->FPS;
    shape->update_center_x(shape->center_x() + dx);

    char buffer[100];
    sprintf(buffer, "%s/%d.png",
        SpecialSetting::special_imgs_root_path[(int)type],
        bitmap_img_ids[0][bitmap_img_id]
    );

    ALLEGRO_BITMAP* bmp = IC->get(buffer);

    double cx = shape->center_x();
    double cy = shape->center_y();
    int w = al_get_bitmap_width(bmp);
    int h = al_get_bitmap_height(bmp);
    float scale = 0.1f;

    shape.reset(new Rectangle{
        cx - (w * scale) / 4,
        cy - (h * scale) / 3,
        cx + (w * scale) / 4,
        cy + (h * scale) / 3
    });
}

void Special::draw() {
    ImageCenter *IC = ImageCenter::get_instance();

    
    // ---------------------------------------------------------
    // ★★★ 若這個 special 正在飛向 UI，專用繪製邏輯 ★★★
    // ---------------------------------------------------------
    if (flying_to_ui) {
        ImageCenter *IC = ImageCenter::get_instance();

        char buffer[100];
        sprintf(buffer, "%s/%d.png",
            SpecialSetting::special_imgs_root_path[(int)type],
            2   // 固定用死亡的 2.png
        );

        ALLEGRO_BITMAP* bmp = IC->get(buffer);
        float w = al_get_bitmap_width(bmp);
        float h = al_get_bitmap_height(bmp);
        float scale = 0.1f;

        al_draw_scaled_bitmap(
            bmp,
            0,0,w,h,
            fly_x - w*scale/2,
            fly_y - h*scale/2,
            w*scale,
            h*scale,
            0
        );

        return;  // 飛行狀態下只畫動畫，不畫原本形體
    }
    // ★ 死亡動畫必須單獨處理，不走一般邏輯
    if (type == SpecialType::TYPHOON) {
        SpecialTyphoon* t = static_cast<SpecialTyphoon*>(this);

        if (t->dying) {
            char buffer[100];
            sprintf(buffer, "%s/%d.png",
                SpecialSetting::special_imgs_root_path[(int)type],
                t->death_frame_index  // = 2.png
            );

            ALLEGRO_BITMAP* bmp = IC->get(buffer);

            float w = al_get_bitmap_width(bmp);
            float h = al_get_bitmap_height(bmp);
            float scale = 0.1f;

            float dw = w * scale;
            float dh = h * scale;

            al_draw_scaled_bitmap(
                bmp,
                0, 0, w, h,
                shape->center_x() - dw / 2,
                shape->center_y() - dh / 2,
                dw, dh,
                0
            );

            return;
        }
    }
    if (type == SpecialType::WIND) {
        SpecialWind* w = static_cast<SpecialWind*>(this);

        if (w->dying) {
            char buffer[100];
            sprintf(buffer, "%s/%d.png",
                SpecialSetting::special_imgs_root_path[(int)type],
                w->death_frame_index  // = 2.png
            );

            ALLEGRO_BITMAP* bmp = IC->get(buffer);

            float w = al_get_bitmap_width(bmp);
            float h = al_get_bitmap_height(bmp);
            float scale = 0.1f;

            float dw = w * scale;
            float dh = h * scale;

            al_draw_scaled_bitmap(
                bmp,
                0, 0, w, h,
                shape->center_x() - dw / 2,
                shape->center_y() - dh / 2,
                dw, dh,
                0
            );

            return;
        }
    }

    // ----------------------------------------------
    // ★ 一般 Special（0/1）
    // ----------------------------------------------
    char buffer[100];
    sprintf(buffer, "%s/%d.png",
        SpecialSetting::special_imgs_root_path[(int)type],
        bitmap_img_ids[0][bitmap_img_id]
    );

    ALLEGRO_BITMAP* bmp = IC->get(buffer);

    float w = al_get_bitmap_width(bmp);
    float h = al_get_bitmap_height(bmp);

    float scale = 0.1f;
    float dw = w * scale;
    float dh = h * scale;

    al_draw_scaled_bitmap(
        bmp,
        0,0,w,h,
        shape->center_x() - dw/2,
        shape->center_y() - dh/2,
        dw, dh,
        0
    );
    
    // Rectangle* r = dynamic_cast<Rectangle*>(shape.get());
    // if (r) {
    //     al_draw_rectangle(r->x1, r->y1, r->x2, r->y2,
    //                     al_map_rgb(0,255,0), 2); // 特殊用綠框
    // }
}
