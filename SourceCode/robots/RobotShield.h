#ifndef ROBOTSHIELD_H_INCLUDED
#define ROBOTSHIELD_H_INCLUDED

#include "Robot.h"
#include "Beam.h"
#include "../shapes/Point.h"

// fixed settings: TowerArcane attributes
class RobotShield : public Robot
{
public:
	RobotShield(const Point &p) : Robot{p, RobotType::SHIELD} {
        HP = 500;

		bitmap_img_ids.clear();
        bitmap_img_ids.emplace_back(std::vector<int>({0, 1})); 
		bitmap_switch_freq = 20;
    }
    Beam *create_beam() override {
		const Point p{shape->center_x(), shape->center_y()};

		double speed = 600;     // 固定子彈速度
		double vx = speed;     // 往右射（敵人方向）
		double vy = 0;          // 不上下移動
		double ay = 0;          // 無重力效果 → 稳定直線射擊

		return new Beam(
			p,
			RobotType::SHIELD,
			RobotSetting::robot_beam_img_path[(int)RobotType::SHIELD], // 你要換成 SHIELD 自己的子彈圖
			vx,
			vy,
			ay,
			2,        // dmg
			1200      // 最大飛行距離
		);
	}

};


#endif