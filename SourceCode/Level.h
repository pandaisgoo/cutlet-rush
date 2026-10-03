#ifndef LEVEL_H_INCLUDED
#define LEVEL_H_INCLUDED

#include <vector>
#include <utility>
#include <tuple>
#include "./shapes/Rectangle.h"
#include "Block.h"

/**
 * @brief The class manages data of each level.
 * @details The class could load level with designated input file and record. The level itself will decide when to create next monster.
 * @see DataCenter::level
 */
class Level
{
public:
	Level() {}
	void init();
	void load_level(int lvl);
	void update();
	void reset(); //重製格子是否可以被放置
	void draw();
	bool is_onroad(const Rectangle &region);
	Rectangle grid_to_region(const Point &grid) const;
	void spawn_student();
	void spawn_special();
	const std::vector<Point> &get_road_path() const
	{ return road_path; }

	// ★★★ 新增：取得特定位置的 Block ★★★
    // 輸入：第幾行 (col), 第幾列 (row)
    // 回傳：該格子的指標，如果超出範圍回傳 nullptr
    Block* get_block(int col, int row);
	void spawn_fixed_robots_level2();
	Point world_to_block(double x, double y) const;
	Point block_center(int col, int row) const;
	int get_level_id() const { return level; }
private:
	/**
	 * @brief Stores the monster's attack route, whose Point is represented in grid format.
	 */
	std::vector<Point> road_path;
	/**
	 * @brief The index of current level.
	 */
	int level;
	/**
	 * @brief Number of grid in x-direction.
	 */
	int grid_w;
	/**
	 * @brief Number of grid in y-direction.
	 */
	int grid_h;


	int student_spawn_counter;
	int special_spawn_counter;
	void spawn_star();

	// ★★★ 新增：儲存所有格子 ★★★
    // 使用 vector of vector 來模擬 2D 陣列: grid[row][col]
    std::vector<std::vector<Block*>> grid;
    
    // 網格設定 (跟 UI.cpp 裡的常數要一致，或是直接搬過來這裡管理)
    const int GRID_ROWS = 5;
    const int GRID_COLS = 8;

	bool is_in_grid(int col, int row) const {
    	return (col >= 0 && col < GRID_COLS && row >= 0 && row < GRID_ROWS);
	}
};

#endif
