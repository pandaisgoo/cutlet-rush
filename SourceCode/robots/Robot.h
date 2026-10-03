#ifndef ROBOT_H_INCLUDED
#define ROBOT_H_INCLUDED

#include "../Object.h"
#include "../shapes/Rectangle.h"
#include <allegro5/bitmap.h>
#include <string>
#include <array>
#include <vector>
#include <queue>
#include "Beam.h"

class Beam;

// fixed settings
enum class RobotType {
	GENERATOR, SHIELD, LIGHT, BOMB, ICE, ADV1, ADV2, ROBOTTYPE_MAX
};

namespace RobotSetting {
	const std::array<std::string, static_cast<int>(RobotType::ROBOTTYPE_MAX)> robot_menu_img_path = {
		"./assets/image/robot/Generator_Menu.png",
        "./assets/image/robot/Shield_Menu.png",
        "./assets/image/robot/Light_Menu.png",
        "./assets/image/robot/Bomb_Menu.png",
        "./assets/image/robot/Ice_Menu.png",
		"0", 
		"0" 
	};
	// ★★★ 新增這段：定義實際機器人圖片的根目錄 ★★★
    static constexpr char robot_imgs_root_path[static_cast<int>(RobotType::ROBOTTYPE_MAX)][60] = {
        "./assets/image/robot/Generator",
        "./assets/image/robot/Shield",
        "./assets/image/robot/Light",
        "./assets/image/robot/Bomb",
        "./assets/image/robot/Ice",
		"./assets/image/robot/Adv1",
		"./assets/image/robot/Adv2"
    };
    // ★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★
	static constexpr char robot_beam_img_path[static_cast<int>(RobotType::ROBOTTYPE_MAX)][60] = {
        "./assets/image/robot/Generator_Beam0.png",   // GENERATOR
        "./assets/image/robot/Shield_Beam0.png",   // SHIELD（目前先用同張）
        "./assets/image/robot/Light_Beam0.png",       // LIGHT
        "./assets/image/robot/Bomb_Beam0.png",        // BOMB
        "./assets/image/robot/Ice_Beam0.png",          // ICE
		"./assets/image/robot/Adv1_Beam0.png",
		"./assets/image/robot/Adv2_Beam0.png"
    };
	const std::array<int, static_cast<int>(RobotType::ROBOTTYPE_MAX)> robot_price = {5, 10, 10, 30, 20, 0, 0};
};

class Robot : public Object
{
public:
	// /**
	//  * @brief Get the ALLEGRO_BITMAP* instance of the full image of a specific RobotType.
	//  */
	// static ALLEGRO_BITMAP *get_bitmap(RobotType type);
	/**
	 * @brief Create a Robot* instance by the type.
	 * @param type the type of a robot.
	 * @param p center point of the robot.
	 */
	static Robot *create_robot(RobotType type, const Point &p);
public:
	Robot(const Point &p, RobotType type);
	virtual ~Robot() {}
	void update();
	virtual bool attack(Object *target);
	void draw();
    int HP;		// robot 血量
	Rectangle get_region() const;
	virtual Beam *create_beam() = 0;
	//virtual const double attack_range() const = 0;
	//RobotType type;
    const std::queue<Point> &get_path() const { return path; }
	RobotType get_type() const { return type; }
	void set_bitmap_img(int id) { bitmap_img_id = id; }
	bool pending_explode = false;
	int explode_timer = 30;
protected:
    std::vector<std::vector<int>> bitmap_img_ids;  // robot 圖片切換
	int bitmap_switch_counter;
	int bitmap_switch_freq;
	int bitmap_img_id;
private:
	/**
	 * @var attack_freq
	 * @brief Robot attack frequency. This variable will be set by its child classes.
	 **
	 * @var counter
	 * @brief Robot attack cooldown.
	 */
	// int attack_freq;
	// int counter;
	ALLEGRO_BITMAP *bitmap;
    std::queue<Point> path;
    RobotType type;
	int beam_cooldown;			// 現在的技能到下一個技能的等待時間
	int beam_cooldown_max;		
	bool ice_in_attack_mode;  
    bool shouldShoot;
};


#endif