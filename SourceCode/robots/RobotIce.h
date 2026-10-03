#ifndef ROBOTICE_H_INCLUDED
#define ROBOTICE_H_INCLUDED

#include "Robot.h"
#include "Beam.h"
#include "../shapes/Point.h"

// fixed settings: TowerArcane attributes
class RobotIce : public Robot
{
public:
	RobotIce(const Point &p) : Robot{p, RobotType::ICE} {
        HP = 1000;

		bitmap_img_ids.clear();
		bitmap_img_ids.push_back({0, 1});
        //bitmap_img_ids.emplace_back(std::vector<int>({0, 1})); 
		bitmap_switch_freq = 60;
    }
    // 子彈要做
    // Beam *create_beam() override {
    //     const Point p{shape->center_x(), shape->center_y()};
    //     const Point dummy{p.x + 1, p.y};

    //     return new Beam(
    //         p,
    //         dummy,
    //         RobotSetting::robot_beam_img_path[(int)RobotType::ICE],
    //         480,
    //         1,     // 冰系通常低傷害
    //         800
    //     );
    // }
    Beam *create_beam() override {
		const Point p{shape->center_x(), shape->center_y()};

		double speed = 500;   // 初速
		double vx = speed;    // x 初速
		double vy = 0.0;      // y 初速
		double ay = 0.0;

		return new Beam(
			p,
			RobotType::ICE,
			RobotSetting::robot_beam_img_path[(int)RobotType::ICE],
			vx,
			vy,
			ay,
			2,      // dmg
			1200    // 飛行距離
		);
	}
};

#endif