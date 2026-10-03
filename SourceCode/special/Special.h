#ifndef SPECIAL_H_INCLUDED
#define SPECIAL_H_INCLUDED

#include "../Object.h"
#include "../shapes/Rectangle.h"
#include "../shapes/Point.h"
#include <vector>
#include <queue>

// fixed settings
enum class SpecialType {
	TYPHOON, WIND, SPECIAL_MAX
};

/**
 * @brief The class of a student (enemies).
 * @details Student inherits Object and takes Rectangle as its hit box.
 */
class Special : public Object
{
public:
	static Special *create_special(SpecialType type, const std::vector<Point> &path);
public:
	Special(const std::vector<Point> &path, SpecialType type, int dmg);
	void update();
	void draw();
	int HP;
	const std::queue<Point> &get_path() const { return path; }
	SpecialType get_type() const { return type; }
	void stop() { v = 0; }
	void recover() { v = old_v; }
	const int &get_dmg() const { return dmg; }
	// 死亡動畫用
    bool dying;
    int death_timer;
    int death_frame_index;   // 直接記住「死亡圖片」在陣列裡的 index
	bool is_counted_dead; // 血條控制
	// typhoon & wind 動畫控制
	bool flying_to_ui = false;
	double ui_target_x = 0;
	double ui_target_y = 0;
	int fly_timer = 0;
	float fly_x, fly_y;   // 動畫用位置
	// ★ 你需要這兩個欄位（如果沒有請加上）
	float start_x, start_y;
protected:
	/**
	 * @var HP
	 * @brief Health point of a monster.
	 **
	 * @var v
	 * @brief Moving speed of a monster.
	 **
	 * @var bitmap_img_ids
	 * @brief The first vector is the Dir index, and the second vector is image id.
	 * @details `bitmap_img_ids[Dir][<ordered_id>]`
	 **
	 * @var bitmap_switch_counter
	 * @brief Counting down for bitmap_switch_freq.
	 * @see Monster::bitmap_switch_freq
	 **
	 * @var bitmap_switch_freq
	 * @brief Number of frames required to change to the next move pose for the current facing direction.
	 * @details The variable is left for child classes to define.
	 * 
	 * @var bitmap_img_id
	 * @brief Move pose of the current facing direction.
	 **
	 * @var dir
	 * @brief Current facing direction.
	 **
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
	
private:
	int dmg;
	SpecialType type;
	std::queue<Point> path;
};

#endif
