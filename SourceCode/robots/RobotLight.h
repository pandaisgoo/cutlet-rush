#ifndef ROBOTLIGHT_H_INCLUDED
#define ROBOTLIGHT_H_INCLUDED

#include "Robot.h"
#include "Beam.h"
#include "../shapes/Point.h"

// fixed settings: TowerArcane attributes
class RobotLight : public Robot
{
public:
	RobotLight(const Point &p) : Robot{p, RobotType::LIGHT} {
        HP = 300;

        bitmap_img_ids.emplace_back(std::vector<int>({0})); 
		bitmap_switch_freq = 120;
    }

	Beam *create_beam() override {
		const Point p{shape->center_x(), shape->center_y()};

		double speed = 500;   // 初速
		double vx = speed;    // x 初速
		double vy = 0.0;      // y 初速
		double ay = 0.0;

		return new Beam(
			p,
			RobotType::LIGHT,
			RobotSetting::robot_beam_img_path[(int)RobotType::LIGHT],
			vx,
			vy,
			ay,
			2,      // dmg
			1200    // 飛行距離
		);
	}
};

#endif