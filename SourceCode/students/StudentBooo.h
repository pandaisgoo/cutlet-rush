#ifndef STUDENTBOOO_H_INCLUDED
#define STUDENTBOOO_H_INCLUDED

#include "Student.h"
#include <vector>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include "../shapes/Rectangle.h"
// fixed settings: StudentNorm attributes
class StudentBooo : public Student
{
public:
	StudentBooo(const std::vector<Point> &path) : Student{path, StudentType::BOOO, 10} {
		HP = 200;        //學生血量
		v = 30;
        old_v = v;

		bitmap_img_ids.emplace_back(std::vector<int>({0, 1})); 
		bitmap_switch_freq = 20;

        exploded = false;       // 是否已爆炸
        explode_timer = 0;      // 爆炸的等待時間
        walked_distance = 0;    // 走了多少距離

	}

    // virtual bool attack(Object *target) override;

    bool exploded;
    int explode_timer;
    float walked_distance;
    ALLEGRO_BITMAP* explode_bmp = nullptr;
    void show_graph();


    // ⭐ 爆炸用的多個 hitbox
    std::vector<Rectangle*> explode_hitboxes;

    bool has_damaged = false;   // 防止重複傷害

    // void create_explode_hitboxes(float draw_w, float draw_h);
    void create_explode_hitboxes();
    void clear_explode_hitboxes();

    // 凍結爆炸圖示的位置
    float explode_cx = 0;
    float explode_cy = 0;
};




#endif