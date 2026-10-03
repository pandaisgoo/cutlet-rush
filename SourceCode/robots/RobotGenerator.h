#ifndef ROBOTGENERATOR_H_INCLUDED
#define ROBOTGENERATOR_H_INCLUDED

#include "Robot.h"
#include "Beam.h"
#include "../shapes/Point.h"
#include <cmath>
#include "../Angle.h"

// fixed settings: TowerArcane attributes
class RobotGenerator : public Robot
{
public:
	RobotGenerator(const Point &p) : Robot{p, RobotType::GENERATOR} {
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

		// --- 1. 設定落點區域（Generator 附近） ---
		double landingX = p.x + rand_range(40, 45);   // 右邊 4~4.5 cm
		double landingY = p.y + rand_range(-30, 30);   // 上下 ±30 範圍

		// --- 2. 飛行時間（短時間 → 拋物線明顯） ---
		double t = rand_range(0.35, 0.55); // 0.35~0.55 秒

		// --- 3. 重力 ---
		double ay = 1400;

		// --- 4. 反推初速 vx, vy ---
		double vx = (landingX - p.x) / t;
		double vy = (landingY - p.y - 0.5 * ay * t * t) / t;

		return new Beam(
			p,
			RobotType::GENERATOR,
			RobotSetting::robot_beam_img_path[(int)RobotType::GENERATOR],
			vx,
			vy,
			ay,
			2,
			1500
		);
	}


};

#endif