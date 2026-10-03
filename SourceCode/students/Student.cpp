#include "Student.h"
#include "StudentNorm.h"
#include "StudentBicycle.h"
#include "StudentDrill.h"
#include "StudentJump.h"
#include "StudentBooo.h"
#include "StudentAdv1.h"

#include "../data/DataCenter.h"
#include "../data/ImageCenter.h"
#include "../Level.h"
#include "../Utils.h"
#include "../shapes/Point.h"
#include "../shapes/Rectangle.h"

#include <allegro5/allegro_primitives.h>
#include <cmath>
#include <string>
#include <iostream>

using namespace std;
void perform_booo_cross_attack(Student* booo);

// void perform_booo_cross_attack(Student* booo)
// {
//     DataCenter* DC = DataCenter::get_instance();
//     float cx = booo->shape->center_x();
//     float cy = booo->shape->center_y();

//     float bomb_range = 200;   // 🔥 十字爆炸距離（可調整）

//     // -----------------------
//     // 攻擊每一台機器人
//     // -----------------------
//     for (Robot* r : DC->robots) {
//         if (r->HP <= 0) continue;

//         float rx = r->shape->center_x();
//         float ry = r->shape->center_y();

//         bool hit =
//             (fabs(rx - cx) <= bomb_range) ||
//             (fabs(ry - cy) <= bomb_range);

//         // 十字判定：同一水平或垂直線上
//         if (hit)
//             r->HP -= 50;   // 可以調整傷害
//     }

//     // -----------------------
//     //（可選）攻擊其他學生
//     // -----------------------
//     for (Student* s : DC->students) {
//         if (s == booo) continue;
//         if (s->HP <= 0) continue;

//         float sx = s->shape->center_x();
//         float sy = s->shape->center_y();

//         bool hit =
//             (fabs(sx - cx) <= bomb_range) ||
//             (fabs(sy - cy) <= bomb_range);

//         if (hit)
//             s->HP -= 50;
//     }
// }
void perform_booo_cross_attack(Student* booo)
{
    // 1️⃣ 型別保護
    if (!booo || booo->get_type() != StudentType::BOOO)
        return;

    StudentBooo* b = static_cast<StudentBooo*>(booo);
    DataCenter* DC = DataCenter::get_instance();

    // 2️⃣ 對每一台 robot
    for (Robot* r : DC->robots) {
        if (!r || r->HP <= 0) continue;

        // 3️⃣ 檢查是否被任一爆炸 hitbox 打到
        // for (Rectangle* hb : b->explode_hitboxes) {
        //     if (hb->overlap(*(r->shape))) {
        //         r->HP -= 50;   // 🔥 爆炸傷害
        //         break;        // ⭐ 一台 robot 只被炸一次
        //     }
        // }
        bool hit_h = b->explode_hitboxes[0]->overlap(*(r->shape));
        bool hit_v = b->explode_hitboxes[1]->overlap(*(r->shape));

        // ⭐ 只允許「橫 or 直」，不允許同時
        if (hit_h ^ hit_v) {
            r->HP -= 50;
        }
    }
}

constexpr char fire_root_path[] = "./assets/image/effect/fire"; 
const int FIRE_FRAME_COUNT = 6; // ★ 請根據你實際拆出來有幾張圖來修改這個數字

void StudentJump::start_jump() {
    if (jumping) return;

    jumping = true;
    jump_delay = 30;               // 在空中停留的 frame
    original_y = shape->center_y();

    bitmap_img_id = 2;             // 跳起來的圖片
    bitmap_switch_counter = bitmap_switch_freq;
}

//----------------------------------------
// Factory
//----------------------------------------
Student* Student::create_student(StudentType type, const vector<Point>& path) {
    switch(type) {
        case StudentType::NORM:    return new StudentNorm(path);
        case StudentType::BICYCLE: return new StudentBicycle(path);
        case StudentType::DRILL:   return new StudentDrill(path);
        case StudentType::JUMP:    return new StudentJump(path);
        case StudentType::BOOO:    return new StudentBooo(path);
        case StudentType::ADV1:    return new StudentAdv1(path);
        case StudentType::STUDENTTYPE_MAX: break;
    }
    GAME_ASSERT(false, "Student type error.");
    return nullptr;
}

//----------------------------------------
// Constructor
//----------------------------------------
Student::Student(const vector<Point> &path, StudentType type, int dmg)
    : type(type)
{
    DataCenter *DC = DataCenter::get_instance();
    shape.reset(new Rectangle{0,0,0,0});
    bitmap_img_id = 0;
    bitmap_switch_counter = 0;
    this->dmg = dmg;

    // 速度相關先設成 0，子類別之後會改掉
    v = 0;
    old_v = 0;

    // 冰緩速初始關閉
    ice_slow = false;
    ice_slow_timer = 0;

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

//----------------------------------------
// Update movement + animation
//----------------------------------------
void Student::update() {
    DataCenter *DC = DataCenter::get_instance();
    ImageCenter *IC = ImageCenter::get_instance();

    // ===============================
    //  WIND 技能：50 frame 的反方向速度
    // ===============================
    if (wind_blown) {
        if (wind_timer > 0) {
            wind_timer--;
        } else {
            // ★ 時間到了 → 恢復正常向左速度
            wind_blown = false;
            v = old_v;  // old_v 是學生原本的速度（Student Constructor 有設）
        }
    }

    // ------------------------
    // 冰子彈緩速效果：時間到就恢復原本速度
    // ------------------------
    if (ice_slow) {
        if (ice_slow_timer > 0) {
            ice_slow_timer--;
        } else {
            ice_slow = false;
            v = old_v;   // 恢復成原本速度
        }
    }


    // ------------------------
    // Drill 特殊行為
    // ------------------------
    if (type == StudentType::DRILL) {
        StudentDrill* d = static_cast<StudentDrill*>(this);

        // ① 鑽地冷卻中 → 不動、不畫
        if (d->drill_cooldown > 0) {
            d->drill_cooldown--;
            return;
        }

        // ② 尚未鑽地
        if (!d->drilling) {

            // ★ 若目前顯示 1.png → 等待 drill_delay_after_frame1 frame 才鑽地
            if (bitmap_img_id == 1) {

                // 一開始看到 1.png → 開始 countdown
                if (d->drill_delay_after_frame1 > 0) {
                    d->drill_delay_after_frame1--;  
                    return;   // ★ 在 1.png 停住，不會消失
                }

                d->drilling = true;
                d->drill_cooldown = 30; // 鑽地 30 frame

                // 記住鑽地前位置
                d->last_x_before_drill = shape->center_x();
                d->last_y_before_drill = shape->center_y();

                // 隱藏
                shape->update_center_x(9999);
                shape->update_center_y(9999);

                // 重設 delay 以便下次循環
                d->drill_delay_after_frame1 = 30;

                return;
            }
        }

        // ③ 鑽地結束 → 重生
        if (d->drilling && d->drill_cooldown == 0) {
            d->drilling = false;

            // 回到圖片 0
            bitmap_img_id = 0;
            bitmap_switch_counter = bitmap_switch_freq;

            // 重生在原位置左邊 30px
            shape->update_center_x(d->last_x_before_drill - 200);
            shape->update_center_y(d->last_y_before_drill);
        }
    }


    // ------------------------
    // Jump 特殊行為（只有 OperationCenter 觸發 start_jump() 才會跳）
    // ------------------------
    if (type == StudentType::JUMP) {
        StudentJump* j = static_cast<StudentJump*>(this);

        // 如果正在跳躍
        if (j->jumping) {

            // 第一次播放跳躍
            if (j->jump_delay == 30) {
                shape->update_center_x(shape->center_x() - 30);
                shape->update_center_y(shape->center_y() - 80); // 跳起
            }

            // 停留在跳躍圖
            if (j->jump_delay > 0) {
                j->jump_delay--;
                return;
            }

            // ----- 跳完落地 -----
            shape->update_center_y(j->original_y);
            shape->update_center_x(shape->center_x() - 30);

            bitmap_img_id = 0;
            bitmap_switch_counter = bitmap_switch_freq;

            j->jumping = false;
            j->jump_delay = 30;

            return;
        }
    }


    // ------------------------
    // Booo 特殊行為：撞到機器人就爆炸
    // ------------------------
    if (type == StudentType::BOOO) {
        StudentBooo* b = static_cast<StudentBooo*>(this);
        auto& robots = DC->robots;

        if (b->exploded) {

            

            //★ countdown
            if (b->explode_timer > 0)
            {
                b->explode_timer--;
                // ⭐ 不能 return，要讓 draw() 有機會畫爆炸圖！
                return;
            }

            // ⭐ 爆炸結束 → 清 hitbox
            b->clear_explode_hitboxes();

            // ★ 爆炸完 → 才真正死亡
            HP = 0;
            return;
        }


        // 🔥 檢查是否撞到任何機器人 → 引爆
        for (Robot* r : robots) {
            if (r->HP <= 0) continue;

            if (shape->overlap(*(r->shape))) {
                // 啟動爆炸
                b->exploded = true;
                b->explode_timer = 30;    
                bitmap_img_id = 1;   

                // ⭐ 凍結爆炸中心（只做一次）
                b->explode_cx = shape->center_x();
                b->explode_cy = shape->center_y();
                
                // ⭐⭐⭐ 在這裡建立爆炸 hitbox（只做一次）
                b->create_explode_hitboxes();

                // 🔥 進行十字爆炸攻擊
                perform_booo_cross_attack(this);

                return;   // 不再繼續走
            }
        }
    }


    // ==========================================
    // ★★★ 燃燒邏輯 ★★★
    // ==========================================
    if (is_burning) {
        // 1. 倒數計時與扣血 (原本的邏輯)
        if (burn_timer > 0) {
            burn_timer--;
            burn_tick++;
            if (burn_tick >= 30) {
                HP -= 5; 
                burn_tick = 0;
            }

            // 2. ★ 火焰動畫切換 ★
            // 每 5 個 frame 換下一張圖 (速度可調)
            if (fire_anim_counter > 0) {
                fire_anim_counter--;
            } else {
                fire_img_id = (fire_img_id + 1) % FIRE_FRAME_COUNT;
                fire_anim_counter = 5; // 重置計時器
            }

        } else {
            // 時間到，火滅了
            is_burning = false;
        }
    }


    // ------------------------
    // 1. Animation switching
    // ------------------------
    if (bitmap_switch_counter > 0)
        bitmap_switch_counter--;
    else {
        bitmap_img_id = (bitmap_img_id + 1) % bitmap_img_ids[0].size();
        bitmap_switch_counter = bitmap_switch_freq;
    }

    // ------------------------
    // 2. Direct movement left
    // ------------------------
    double dx = -v / DC->FPS;   // ← 每 frame 向左位移
    shape->update_center_x(shape->center_x() + dx);

    // ------------------------
    // 3. Update hitbox (no scale)
    // ------------------------
    char buffer[100];
    sprintf(buffer, "%s/%d.png",
        StudentSetting::student_imgs_root_path[(int)type],
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

    // =========================================================
    // ★★★ 4. 死亡判定與盾牌變身 (Shield Break Logic) ★★★
    // =========================================================
    if (HP <= 0) {
        
        // 如果我是「盾牌學生 (ADV1)」，血量歸零時不會死，而是變身為「普通學生 (NORM)」
        if (type == StudentType::ADV1) {
            
            // A. 變身為普通學生
            type = StudentType::NORM;
            
            // B. 補滿血 (給予普通學生的血量 10)
            HP = 10; 
            max_hp = 10; // 更新最大血量，確保血條正確

            // C. 恢復速度 (可選)
            // 如果原本 ADV1 比較慢，這裡可以設回 NORM 的速度
            // v = 100; // 或 StudentSetting 中定義的速度
            // old_v = v;

            // D. 重置動畫狀態
            // 強制切換回第 0 張圖，避免剛好卡在奇怪的動作
            bitmap_img_id = 0;
            bitmap_switch_counter = bitmap_switch_freq;

            // Debug Log
            // printf("Shield Broken! Transformed to Normal Student.\n");

        } 
        else {
            // 如果不是盾牌學生 (例如已經是 NORM, DRILL 等)，那就真的死了
            
            // 如果是 BOOO，它的死亡邏輯在上面自己處理了 (HP=0 且 exploded=true)
            // 所以這裡主要是針對其他類型的真正死亡
            
            // 設定為「待刪除」，讓 Game::update 或 Level::update 把它從 vector 移除
            // 注意：這裡假設你的 DataCenter 或 Game loop 會檢查 HP <= 0 來刪除物件
            // 如果你的刪除邏輯是寫在這裡，請加上 delete 標記
        }
    }
}

//----------------------------------------
// Draw
//----------------------------------------
void Student::draw() {
    ImageCenter *IC = ImageCenter::get_instance();

    if (type == StudentType::BOOO) {
    StudentBooo* b = static_cast<StudentBooo*>(this);
    if (b->exploded && b->explode_timer == 0)
        return;   // 已經爆炸完成 → 不畫
    }


    if (type == StudentType::DRILL) {
    StudentDrill* d = static_cast<StudentDrill*>(this);
    if (d->drilling || d->drill_cooldown > 0)
        return; // 鑽地期間不畫
    }

    // =================================================
    // 1️⃣ BOOO：爆炸狀態
    // =================================================
    if (type == StudentType::BOOO) {

        StudentBooo* b = static_cast<StudentBooo*>(this);

        // 爆炸結束 → 完全不畫
        if (b->exploded && b->explode_timer <= 0)
            return;

        // ---------------------------
        // 正在爆炸：畫 explode.png
        // ---------------------------
        if (b->exploded) {
            

            // 確保 explode 圖只載入一次
            if (!b->explode_bmp) {
                b->explode_bmp = IC->get(
                    "./assets/image/student/Booo/explode.png"
                );
            }

            const float scale = 0.25f;

            float w = al_get_bitmap_width(b->explode_bmp);
            float h = al_get_bitmap_height(b->explode_bmp);

            float draw_w = w * scale;
            float draw_h = h * scale;

            float cx = shape->center_x();
            float cy = shape->center_y();

            // 畫爆炸圖
            al_draw_scaled_bitmap(
                b->explode_bmp,
                0, 0, w - 80, h,
                cx - draw_w / 2 - 80,
                cy - draw_h / 2,
                draw_w,
                draw_h,
                0
            );

            // 🔍 Debug：畫「實際用來判斷的爆炸 hitbox」
            // for (Rectangle* hb : b->explode_hitboxes) {
            //     al_draw_rectangle(
            //         hb->x1, hb->y1,
            //         hb->x2, hb->y2,
            //         al_map_rgb(255, 255, 0),
            //         2
            //     );
            // }

            return; // ⭐ 爆炸時，不畫原本學生
        }
    }


    char buffer[100];
    sprintf(buffer, "%s/%d.png",
        StudentSetting::student_imgs_root_path[(int)type],
        bitmap_img_ids[0][bitmap_img_id]
    );

    ALLEGRO_BITMAP* bmp = IC->get(buffer);

    float scale = 0.1f;   // ★★★ 調整大小就在這裡

    float w = al_get_bitmap_width(bmp);
    float h = al_get_bitmap_height(bmp);

    float draw_w = w * scale;
    float draw_h = h * scale;

    al_draw_scaled_bitmap(
        bmp,
        0, 0, w, h,    // 原圖
        shape->center_x() - draw_w / 2,
        shape->center_y() - draw_h / 2,
        draw_w,
        draw_h,
        0
    );

    // ==========================================
    // ★★★ 畫出火焰 ★★★
    // ==========================================
    if (is_burning) {
        // 組合路徑： ./assets/image/effect/fire/0.gif
        char buffer[100];
        sprintf(buffer, "%s/%d.png", fire_root_path, fire_img_id);

        ALLEGRO_BITMAP *fire = IC->get(buffer);
        if (fire) {
            int fw = al_get_bitmap_width(fire);
            int fh = al_get_bitmap_height(fire);
            
            // 畫在學生下面
            al_draw_scaled_bitmap(
                fire,
                0, 0, fw, fh,
                shape->center_x() - 20, shape->center_y() - 20, // 位置
                70, 70, // 大小
                0
            );
        }
        // ★★★ 加入除錯訊息 ★★★
        if (!fire) {
            std::cout << "Failed to load fire image: " << buffer << std::endl;
        }
    }


// Rectangle* r = dynamic_cast<Rectangle*>(shape.get());
// if (r) {
//     al_draw_rectangle(
//         r->x1, r->y1,
//         r->x2, r->y2,
//         al_map_rgb(255, 0, 0),    // Hitbox 顏色
//         2                         // 線條粗度
//     );
// }

    // ==========================
    // ★ 冰凍特效：淺藍透明長方形
    // ==========================
    if (ice_slow) {
        Rectangle* r = dynamic_cast<Rectangle*>(shape.get());
        if (r) {
            al_draw_filled_rectangle(
                r->x1, r->y1 - 5,
                r->x2, r->y2,
                al_map_rgba(100, 180, 255, 80)   // 淺藍透明
            );
        }
    }


}


void Student::chanege_speed() {
    DataCenter* DC = DataCenter::get_instance();

    // 第一次被冰到時，記一下原本速度（如果 old_v 還沒被設過）
    if (old_v == 0)
        old_v = v;

    ice_slow = true;

    // 持續時間：這裡先設 1 秒（= FPS frame）
    // 你可以改成 2 * DC->FPS 變成 2 秒之類的
    ice_slow_timer = DC->FPS;

    // 被冰之後的速度（你原本寫 60，我就沿用）
    v = 20;
}



void StudentBooo::create_explode_hitboxes() {

    // ✅ 確保 explode_bmp 已載入
    if (!explode_bmp) {
        explode_bmp = ImageCenter::get_instance()
            ->get("./assets/image/student/Booo/explode.png");
    }

    
    float scale = 0.25f;
    float w = al_get_bitmap_width(explode_bmp);
    float h = al_get_bitmap_height(explode_bmp);

    float draw_w = w * scale;
    float draw_h = h * scale;

    float cx = explode_cx;
    float cy = explode_cy;

    explode_hitboxes.push_back(
        new Rectangle(
            cx - draw_w / 3 - 70,
            cy - draw_h / 5,
            cx + draw_w / 3 - 70,
            cy + draw_h / 5
        )
    );

    explode_hitboxes.push_back(
        new Rectangle(
            cx - draw_w / 6 - 70,
            cy - draw_h / 2,
            cx + draw_w / 6 - 70,
            cy + draw_h / 2
        )
    );
}

void StudentBooo::clear_explode_hitboxes() {
    for (Rectangle* r : explode_hitboxes)
        delete r;
    explode_hitboxes.clear();
}
