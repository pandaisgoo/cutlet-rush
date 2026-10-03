#ifndef BLOCK_H_INCLUDED
#define BLOCK_H_INCLUDED

#include "shapes/Point.h"
#include "shapes/Rectangle.h"
#include "robots/Robot.h"
#include <allegro5/allegro.h>

class Block
{
public:
    // 建構子：傳入格子的座標 (x, y) 和大小 (w, h)
    Block(int x, int y, int w, int h);
    
    void draw();
    
    // 檢查這一格是否已經有機器人
    bool has_robot() const { return robot != nullptr; }
    
    // 取得這一格上面的機器人
    Robot* get_robot() { return robot; }
    
    // 設定這一格的機器人
    void set_robot(Robot* r) { robot = r; }
    
    // 移除這一格的機器人 (例如機器人死了)
    void clear_robot() { robot = nullptr; }

    // 取得格子的判定範圍 (給滑鼠偵測用)
    Rectangle get_region() const { return shape; }

    // 設定顏色 (為了畫出西洋棋盤效果)
    void set_color(ALLEGRO_COLOR c) { color = c; }

private:
    int x, y, w, h;
    Rectangle shape;       // 格子的矩形範圍
    Robot* robot = nullptr; // 這一格上面站著誰 (nullptr 代表空地)
    ALLEGRO_COLOR color;   // 格子的顏色
};

#endif