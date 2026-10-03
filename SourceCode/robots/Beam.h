#ifndef BEAM_H_INCLUDED
#define BEAM_H_INCLUDED

#include "../Object.h"
#include <allegro5/bitmap.h>
#include <string>
#include <vector>

enum class RobotType; 

/**
 * @brief The bullet shot from Tower.
 * @see Tower
 */
class Beam : public Object
{
public:
    Beam(const Point &p, RobotType type, const std::string &path, double vx, double vy, double ay, int dmg, double fly_dist);
    void update();
    void draw();
    const double &get_fly_dist() const { return fly_dist; }
    void set_fly_dist(double dist) { fly_dist = dist; }
    // ★★★ 新增：收集函式 ★★★
    void collect() { is_collected = true; }
    // ★★★ 新增：檢查是否正在飛向計分板 ★★★
    bool get_is_collected() const { return is_collected; }
    const int &get_dmg() const { return dmg; }
    RobotType get_type() const { return type; }
    // 旋轉子彈
    double rotation;
    double rotation_v;
private:
    double start_x;         // robot x初始位置 (generator 子彈要落在它的右下角)
    double start_y;         // robot y初始位置 (generator 子彈要落在它的右下角)
    double vx;              // 初速 x
    double vy;              // 初速 y
    double ay;              // 重力加速度 
    double fly_dist;        // 飛行距離
    int dmg;                
    RobotType type;         // 不同機器人有不同子彈效果
    ALLEGRO_BITMAP *bitmap; // 未縮放過的子彈圖

    std::vector<int> frame_ids;  // total子彈照片數量
    int frame_id;                // 對應第幾張子彈照片
    int frame_counter;           // 子彈倒計時
    int frame_freq;              // 每隔多久生出下一張的照片

    bool is_collected; // 是否已被收集
    bool clicked; // only for generator
    int lifetime;

};

#endif