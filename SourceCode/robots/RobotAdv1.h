#ifndef ROBOTADV1_H_INCLUDED
#define ROBOTADV1_H_INCLUDED

#include "Robot.h"
#include "Beam.h"
#include "../shapes/Point.h"
#include <cmath>
#include "../Angle.h"
#include "../data/DataCenter.h"

// fixed settings: TowerArcane attributes
class RobotAdv1 : public Robot
{
public:
	RobotAdv1(const Point &p) : Robot{p, RobotType::ADV1} {
        HP = 500;

        bitmap_img_ids.emplace_back(std::vector<int>({0})); // generator只有一張圖
		bitmap_switch_freq = 20;
		bitmap_switch_counter = 0;
		min_interval = 150;   // 最短 150 frame（2.5 秒）
        max_interval = 450;   // 最長 450 frame（7.5 秒）
    }

	int min_interval; // 產生新的子彈的最小間隔長度
	int max_interval; // 產生新的子彈的最大間隔長度

    Beam *create_beam() override {
        const Point p{shape->center_x(), shape->center_y()};
        DataCenter* DC = DataCenter::get_instance();

        double base_angle = 0;
        double speed = 600;

        for (int i = 0; i < 5; i++) {
            double angle = base_angle + i * 72;
            double rad = angle * M_PI / 180.0;

            double vx = speed * cos(rad);
            double vy = speed * sin(rad);

            Beam* b = new Beam(
                p,
                RobotType::ADV1,
                RobotSetting::robot_beam_img_path[(int)RobotType::ADV1],  // ★ 使用五邊形子彈圖片
                vx, vy,
                0,
                5,     // 傷害可自由改
                1500
            );

            // 可選：每顆讓旋轉速度不同
            b->rotation_v = 5 + i * 10; // 5,7,9,11,13 度/幀

            DC->beams.emplace_back(b);
        }

        // Dummy beam（避免 attack() nullptr）
        Beam* dummy = new Beam(
            p,
            RobotType::ADV1,
            RobotSetting::robot_beam_img_path[(int)RobotType::ADV1],
            0,0,0,0,0
        );
        dummy->set_fly_dist(0);
        return dummy;
    }



};

#endif