#ifndef STUDENT_H_INCLUDED
#define STUDENT_H_INCLUDED

#include "../Object.h"
#include "../shapes/Rectangle.h"
#include "../shapes/Point.h"
#include <vector>
#include <queue>

// fixed settings
enum class StudentType {
    NORM, BICYCLE, DRILL, JUMP, BOOO, ADV1, STUDENTTYPE_MAX
};

namespace StudentSetting {
    static constexpr char student_imgs_root_path[static_cast<int>(StudentType::STUDENTTYPE_MAX)][60] = {
        "./assets/image/student/Norm",
        "./assets/image/student/Bicycle",
        "./assets/image/student/Drill",
        "./assets/image/student/Jump",
        "./assets/image/student/Booo",
        "./assets/image/student/Adv1"
    };
}

/**
 * @brief The class of a student (enemies).
 * @details Student inherits Object and takes Rectangle as its hit box.
 */
class Student : public Object
{
public:
    static Student *create_student(StudentType type, const std::vector<Point> &path);
public:
    Student(const std::vector<Point> &path, StudentType type, int dmg);
    void update();
    void draw();

    // ★★★ 讓學生著火的函式 (Ver 2 功能) ★★★
    void set_burning(bool burning) { 
        is_burning = burning; 
        if (burning) {
            burn_timer = 180; // 燒 3 秒 (60 FPS * 3)
            burn_tick = 0;    // 重置傷害計時
        }
    }

    int HP;
    int max_hp; // ★★★ 新增：最大血量 (用於變身重置和血條計算) ★★★
    const std::queue<Point> &get_path() const { return path; }
    StudentType get_type() const { return type; }
    void stop() { v = 0; }
    void recover() { v = old_v; }
    void chanege_speed();
    const int &get_dmg() const { return dmg; }
    bool is_counted_dead = false;
    // virtual bool attack(Object *target);

    // ==========================
    // ★★★ WIND skill getters/setters (從 Ver 1 整合進來) ★★★
    // ==========================

    // 回傳學生是否正在 WIND 狀態
    bool getWindBlown() const { return wind_blown; }

    // 設定 WIND 狀態（true=被吹, false=恢復）
    void setWindBlown(bool val) { wind_blown = val; }

    // 回傳剩餘 WIND 計時器
    int getWindTimer() const { return wind_timer; }

    // 設定剩餘 WIND 計時器
    void setWindTimer(int t) { wind_timer = t; }

    // 回傳舊速度
    double getOldV() const { return old_v; }

    // 設定舊速度
    void setOldV(double v0) { old_v = v0; }

    // 回傳目前速度
    double getV() const { return v; }

    // 設定目前速度
    void setV(double nv) { v = nv; }

protected:
    /**
     * @var HP
     * @brief Health point of a monster.
     *
     * @var v
     * @brief Moving speed of a monster.
     *
     * @var bitmap_img_ids
     * @brief The first vector is the Dir index, and the second vector is image id.
     * @details `bitmap_img_ids[Dir][<ordered_id>]`
     *
     * @var bitmap_switch_counter
     * @brief Counting down for bitmap_switch_freq.
     * @see Monster::bitmap_switch_freq
     *
     * @var bitmap_switch_freq
     * @brief Number of frames required to change to the next move pose for the current facing direction.
     * @details The variable is left for child classes to define.
     * * @var bitmap_img_id
     * @brief Move pose of the current facing direction.
     *
     * @var dir
     * @brief Current facing direction.
     *
     * @var path
     * @brief The walk path of a monster, represented in grid format.
     * @see Level::grid_to_region(const Point &grid) const
    */
    int v;
    int old_v;
    std::vector<std::vector<int>> bitmap_img_ids;
    int bitmap_switch_counter;
    int bitmap_switch_freq;
    int bitmap_img_id;

    // ★★★ 燃燒相關變數 (Ver 2 功能) ★★★
    bool is_burning = false;      // 預設為 false，需要時再設為 true
    int burn_timer = 0;           // 燃燒剩餘時間
    int burn_tick = 0;            // 用來控制扣血頻率 (例如每 0.5 秒扣一次)
    int fire_img_id = 0;          // 目前播放到第幾張火焰圖
    int fire_anim_counter = 0;    // 控制火焰動畫速度

    // ==== 冰子彈緩速狀態 ====
    bool ice_slow = false;     // 是否正被冰緩速
    int  ice_slow_timer = 0;   // 緩速剩餘 frame 數

    // ★★★ 風吹狀態變數 (從 Ver 1 整合進來) ★★★
    bool wind_blown = false;    // 是否正在被風吹效果影響
    int  wind_timer = 0;        // 還剩多少 frame 要恢復

private:
    int dmg;
    StudentType type;
    std::queue<Point> path;
};

#endif