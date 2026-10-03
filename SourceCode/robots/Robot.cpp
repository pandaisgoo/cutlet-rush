#include "Robot.h"
#include "RobotGenerator.h"
#include "RobotShield.h"
#include "RobotLight.h"
#include "RobotBomb.h"
#include "RobotIce.h"
#include "RobotAdv1.h"
#include "RobotAdv2.h"
#include "../Angle.h"

#include "../Utils.h"
#include "../students/Student.h"
#include "../special/Special.h"

#include "../shapes/Circle.h"
#include "../shapes/Rectangle.h"
#include "../Level.h"

#include "../data/DataCenter.h"
#include "../data/ImageCenter.h"
#include "../data/SoundCenter.h"
#include "../data/FontCenter.h"
#include <allegro5/bitmap_draw.h>
#include <allegro5/allegro_primitives.h>
#include <cmath>
#include <string>

using namespace std;

// fixed settings
namespace RobotSetting {
    //搬到 robot.h
    /*
    static constexpr char robot_imgs_root_path[static_cast<int>(RobotType::ROBOTTYPE_MAX)][60] = {
        "./assets/image/robot/Generator",
        "./assets/image/robot/Shield",
        "./assets/image/robot/Light",
        "./assets/image/robot/Bomb",
        "./assets/image/robot/Ice"
    };*/
    // 聲音最後做
    constexpr char beam_attack_sound_path[] = "./assets/sound/Arrow.wav";
};

Robot*
Robot::create_robot(RobotType type, const Point &p) {
    switch(type) {
        case RobotType::GENERATOR: {
            return new RobotGenerator(p);
        } case RobotType::SHIELD: {
            return new RobotShield(p);
        } case RobotType::LIGHT: {
            return new RobotLight(p);
        } case RobotType::BOMB: {
            return new RobotBomb(p);
        } case RobotType::ICE: {
            return new RobotIce(p);
        }  case RobotType::ADV1: {
            return new RobotAdv1(p);
        }  case RobotType::ADV2: {
            return new RobotAdv2(p);
        } case RobotType::ROBOTTYPE_MAX: {}
    }
    GAME_ASSERT(false, "robot type error.");
    return nullptr;
}

/**
 * @param p center point (x, y).
 * @param attack_range any monster inside this number would trigger attack.
 * @param attack_freq period for tower to attack.
 * @param type tower type.
*/
Robot::Robot(const Point &p, RobotType type)
    : type(type)
{
    ImageCenter *IC = ImageCenter::get_instance();

    // 1. 初始化動畫（實際的 frame 在子類別建構子才會決定）
    bitmap_img_id = 0;
    bitmap_switch_counter = 0;
    beam_cooldown = 0;
    beam_cooldown_max = 0;
    HP = 1000;

    bitmap_img_ids.clear();
    bitmap_img_ids.push_back({0});  
    bitmap_switch_freq = 0;

    ice_in_attack_mode = false;  
    shouldShoot = false;

    // 2. 暫時用 type 的 0.png 當初始化圖片
    char buffer[100];
    sprintf(buffer, "%s/%d.png",
        RobotSetting::robot_imgs_root_path[(int)type],
        0
    );
    ALLEGRO_BITMAP* bmp = IC->get(buffer);
    bitmap = bmp;

    int w = al_get_bitmap_width(bmp);
    int h = al_get_bitmap_height(bmp);

    // 3. 設置初始位置
    float scale = 0.1f;  // 與 draw() 使用的 scale 一樣

    shape.reset(new Rectangle{
        p.x - (w * scale) / 3,
        p.y - (h * scale) / 2,
        p.x + (w * scale) / 3,
        p.y + (h * scale) / 2
    });
}

/**
 * @brief Update attack cooldown and detect if the tower could make an attack.
 * @see Tower::attack(Object *target)
*/
void Robot::update() {

    DataCenter *DC = DataCenter::get_instance();
    ImageCenter *IC = ImageCenter::get_instance();

    // 取得當前位置
    double robotX = shape->center_x();
    double robotY = shape->center_y();

    // ==========================================
    // 1. 冷卻時間倒數 (通用)
    // ==========================================
    if (beam_cooldown > 0) {
        beam_cooldown--;
    } 

    // ==========================================
    // 2. 特殊機器人邏輯 (不需要敵人也能運作)
    // ==========================================
    
    // (A) GENERATOR: 時間到就產星星
    if (type == RobotType::GENERATOR) {
        if (beam_cooldown <= 0) {
            RobotGenerator* g = static_cast<RobotGenerator*>(this);
            DC->beams.emplace_back(create_beam());
            beam_cooldown = rand_int(g->min_interval, g->max_interval);
        }
        // Generator 更新完直接跳去跑動畫
        goto UPDATE_ANIMATION; 
    }

    // (B) BOMB: 倒數計時與爆炸
    if (type == RobotType::BOMB) {
        
        // 如果 HP 已經歸零 (被打爆)，就不執行爆炸邏輯，等著被清理
        if (this->HP <= 0) return;

        RobotBomb* bomb = static_cast<RobotBomb*>(this);
        
        // 階段 1: 還沒爆炸，進行倒數
        if (!bomb->exploding) {
            if (bomb->explode_timer > 0) {
                bomb->explode_timer--;
                
                // (選用) 倒數快到時加快閃爍，增加緊張感
                if (bomb->explode_timer < 60) bitmap_switch_freq = 5;
            } 
            else {
                // ★★★ 時間到！觸發爆炸 ★★★
                bomb->exploding = true;
                bomb->explode_timer = 20; // 爆炸動畫時間
                
                // 讓全場學生著火
                for (Student *s : DC->students) {
                    s->set_burning(true);
                    // s->HP -= 500; // 瞬間傷害 (選用)
                }
                
                // 播放音效 (選用)
                // SoundCenter::get_instance()->play(RobotSetting::beam_attack_sound_path, ALLEGRO_PLAYMODE_ONCE);
            }
        }
        // 階段 2: 正在爆炸 (顯示爆炸圖)
        else {
            if (bomb->explode_timer > 0) {
                bomb->explode_timer--; // 停留一下顯示爆炸特效
            } else {
                // 動畫播完，自殺
                this->HP = 0; 
            }
        }
        
        // 炸彈邏輯結束，跳去更新動畫 (確保顯示爆炸圖)
        goto UPDATE_ANIMATION;
    }

    // ==========================================
    // 3. 攻擊型機器人 (需要找敵人)
    // ==========================================
    if (beam_cooldown <= 0) {
        shouldShoot = false; // 每幀重置
        
        // (A) 檢查學生
        for (Student *student : DC->students) {
            double studentX = student->shape->center_x();
            double studentY = student->shape->center_y();

            // ICE 特殊判定
            if (type == RobotType::ICE) {
                if ((studentX - robotX) >= 0 && std::abs(robotY - studentY) <= 20) {
                    bitmap_img_id = 1;
                    ice_in_attack_mode = true;
                    if (bitmap_img_id == 1) shouldShoot = true;
                } else {
                    bitmap_img_id = 0;
                    ice_in_attack_mode = false;
                    shouldShoot = false;
                }
                if (!ice_in_attack_mode) continue;
            }
            // 其他射擊機器人 (LIGHT, SHIELD 等)
            else {
                if ((studentX - robotX) < 0) continue;
                if (std::abs(robotY - studentY) > 20) continue;
                shouldShoot = true;
            }

            // 執行射擊
            if (shouldShoot) {
                if (attack(student)) {
                    beam_cooldown = 18;
                    break; // 一次只打一個
                }
            }
        }
        
        // (B) 檢查 Special (如果沒打到學生，再檢查颱風)
        if (!shouldShoot) { 
             for (Special* sp : DC->specials) {
                if (sp->HP <= 0) continue;
                double sx = sp->shape->center_x();
                double sy = sp->shape->center_y();

                if ((sx - robotX) < 0) continue;
                if (std::abs(sy - robotY) > 20) continue;

                if (attack(sp)) {
                    beam_cooldown = 18;
                    break;
                }
             }
        }
    }

// 標籤：動畫更新區塊
UPDATE_ANIMATION:

    // ==========================================
    // 4. 動畫更新 (Animation)
    // ==========================================
    
    // ★★★ BOMB 特殊處理：如果是爆炸狀態，鎖定在爆炸圖 (1.png) ★★★
    if (type == RobotType::BOMB) {
        RobotBomb* bomb = static_cast<RobotBomb*>(this);
        if (bomb->exploding) {
             bitmap_img_id = 1; // 強制鎖定在爆炸圖
        } else {
             bitmap_img_id = 0; // 倒數時顯示正常圖
        }
        // 不跑下面的自動循環，避免切回 0
    }
    // ICE 特殊處理
    else if (type == RobotType::ICE) {
        if (!ice_in_attack_mode) {
            bitmap_img_id = 0;
        } else {
            if (bitmap_switch_counter > 0) bitmap_switch_counter--;
            else {
                bitmap_img_id = (bitmap_img_id + 1) % bitmap_img_ids[0].size();
                bitmap_switch_counter = bitmap_switch_freq;
            }
        }
    }
    // 其他機器人：正常循環播放
    else {
        if (bitmap_switch_counter > 0) bitmap_switch_counter--;
        else {
            bitmap_img_id = (bitmap_img_id + 1) % bitmap_img_ids[0].size();
            bitmap_switch_counter = bitmap_switch_freq;
        }
        
        // RobotShield：血量變低時換照片
        if (type == RobotType::SHIELD) {
            if (HP <= 400 && bitmap_img_ids[0].size() > 1)
                bitmap_img_id = 1;   
            else
                bitmap_img_id = 0;   
        }
    }

    // ==========================================
    // 5. 圖片路徑計算
    // ==========================================
    
    int vector_index = 0;
    // 針對 RobotBomb 的相容性處理 (如果用 emplace_back 的話 index 為 1)
    // 如果你已經改用 bitmap_img_ids[0] = {0, 1}，這裡維持 0 即可
    if (type == RobotType::BOMB && bitmap_img_ids.size() > 1) {
        vector_index = 1;
    }
    
    // 安全檢查
    if (bitmap_img_ids.empty() || bitmap_img_ids[vector_index].empty()) return;
    if (bitmap_img_id >= bitmap_img_ids[vector_index].size()) bitmap_img_id = 0;

    char buffer[100];
    sprintf(buffer, "%s/%d.png",
            RobotSetting::robot_imgs_root_path[(int)type],
            bitmap_img_ids[vector_index][bitmap_img_id]);

    // 這裡不更新 shape，因為 Hitbox 固定就好，不需要每一幀都 new
}


bool Robot::attack(Object *target)
{
    DataCenter* DC = DataCenter::get_instance();
    SoundCenter* SC = SoundCenter::get_instance();

    // ⭐ 允許 target 是 student 或 special
    Student* stu = dynamic_cast<Student*>(target);
    Special* sp  = dynamic_cast<Special*>(target);

    if (!stu && !sp)
        return false;     // 不是可攻擊對象

    // ⭐ 發射子彈（beam）照舊
    DC->beams.emplace_back(create_beam());

    // SC->play(RobotSetting::beam_attack_sound_path, ALLEGRO_PLAYMODE_ONCE);
    return true;
}


void
Robot::draw() {
    al_draw_filled_circle(shape->center_x(), shape->center_y(), 5, al_map_rgb(255, 0, 0));

    ImageCenter *IC = ImageCenter::get_instance();

    char buffer[100];
    sprintf(buffer, "%s/%d.png",
        RobotSetting::robot_imgs_root_path[(int)type],
        bitmap_img_ids[0][bitmap_img_id]
    );

    ALLEGRO_BITMAP* bmp = IC->get(buffer);

    float scale = 0.1f;   // ★★★ 調整大小就在這裡

    float w = al_get_bitmap_width(bmp);
    float h = al_get_bitmap_height(bmp);

    float draw_w = w * scale;
    float draw_h = h * scale;

    // 繪製圖片
    al_draw_scaled_bitmap(
        bmp,
        0, 0, w, h,    // 原圖
        shape->center_x() - draw_w / 2,
        shape->center_y() - draw_h / 2,
        draw_w,
        draw_h,
        0
    );


    // ⭐ 加入 hitbox（矩形框）
    // float left   = shape->center_x() - draw_w / 2;
    // float right  = shape->center_x() + draw_w / 2;
    // float top    = shape->center_y() - draw_h / 2;
    // float bottom = shape->center_y() + draw_h / 2;
    // ⭐ hitbox 縮放比例
    // float hitbox_scale = 0.6f;

    // // 原本圖片的一半大小
    // float half_w = draw_w / 2;
    // float half_h = draw_h / 2;

    // // 縮小後的一半
    // float hit_half_w = half_w * hitbox_scale;
    // float hit_half_h = half_h * hitbox_scale;

    // // hitbox 四邊
    // float left   = shape->center_x() - hit_half_w;
    // float right  = shape->center_x() + hit_half_w;
    // float top    = shape->center_y() - hit_half_h;
    // float bottom = shape->center_y() + hit_half_h;


    // al_draw_rectangle(
    //     left, top, right, bottom,
    //     al_map_rgb(255, 0, 0),
    //     2
    // );

    // ==========================================
    // ★★★ 新增：炸彈倒數計時顯示 (右上角) ★★★
    // ==========================================
    if (type == RobotType::BOMB) {
        RobotBomb* bomb = static_cast<RobotBomb*>(this);

        // 只有在「未爆炸」且「倒數中」的時候才畫
        if (!bomb->exploding && bomb->explode_timer > 0) {
            
            FontCenter *FC = FontCenter::get_instance();

            // 1. 計算位置 (顯示在機器人右上角)
            // 假設機器人大小約 40x40，我們往右上偏移
            float timer_x = shape->center_x() + 50; 
            float timer_y = shape->center_y() - 10;
            float radius = 20; // 圓的大小

            // 2. 計算進度比例 (0.0 ~ 1.0)
            // 假設總時間是 180 frames (3秒)
            float max_time = 180.0f; 
            float ratio = (float)bomb->explode_timer / max_time;

            // 3. 畫底圓 (深紅色背景)
            al_draw_filled_circle(timer_x, timer_y, radius, al_map_rgb(50, 0, 0));

            // 4. 畫進度條 (亮紅色扇形)
            // -ALLEGRO_PI / 2 代表從 12 點鐘方向開始
            // 2 * ALLEGRO_PI * ratio 代表畫多少角度
            al_draw_filled_pieslice(
                timer_x, timer_y, 
                radius, 
                -ALLEGRO_PI / 2,        // 起始角度 (上方)
                2 * ALLEGRO_PI * ratio, // 掃過的角度
                al_map_rgb(255, 50, 50)
            );
            
            // (選用) 畫個白框讓它明顯一點
            al_draw_circle(timer_x, timer_y, radius, al_map_rgb(255, 255, 255), 2);

            // 5. 畫數字 (秒數)
            // 將 frames 換算成秒數 (無條件進位)
            int seconds = (int)ceil(bomb->explode_timer / 60.0);
            
            al_draw_textf(
                FC->courier_new[FontSize::SMALL], // 確保你有這個字體
                al_map_rgb(255, 255, 255),        // 白色字
                timer_x, 
                timer_y - 6,                      // 稍微往上修一點置中
                ALLEGRO_ALIGN_CENTRE, 
                "%d", 
                seconds
            );
        }
    }
    // ==========================================
}

/**
 * @brief Get the area of the tower, and return with a Rectangle object.
*/
Rectangle
Robot::get_region() const {
    int w = al_get_bitmap_width(bitmap);
    int h = al_get_bitmap_height(bitmap);
    return {
        shape->center_x() - w/2,
        shape->center_y() - h/2,
        shape->center_x() - w/2 + w,
        shape->center_y() - h/2 + h
    };
}