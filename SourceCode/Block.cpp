#include "Block.h"
#include <allegro5/allegro_primitives.h>

Block::Block(int x, int y, int w, int h) {
    this->x = x;
    this->y = y;
    this->w = w;
    this->h = h;
    this->shape = Rectangle(x, y, x + w, y + h);
    this->robot = nullptr;
    this->color = al_map_rgb(200, 200, 200); // 預設灰色
}

void Block::draw() {
    // 畫出格子的底色
    //al_draw_filled_rectangle(x, y, x + w, y + h, color);
    
    // 畫個邊框讓格子明顯一點 (選用)
    al_draw_rectangle(x, y, x + w, y + h, al_map_rgb(0, 0, 0), 1);
}