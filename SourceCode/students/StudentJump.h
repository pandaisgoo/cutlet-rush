#ifndef STUDENTJUMP_H_INCLUDED
#define STUDENTJUMP_H_INCLUDED

#include "Student.h"

// fixed settings: StudentNorm attributes
class StudentJump : public Student
{
public:
	StudentJump(const std::vector<Point> &path) : Student{path, StudentType::JUMP, 4} {
		HP = 30;        //學生血量
		v = 50;
        old_v = v;

		bitmap_img_ids.emplace_back(std::vector<int>({0, 1, 2})); 
		bitmap_switch_freq = 20;

        jump_delay = 30;   // 跳到第三張後停 10 frame
        jumping = false;
	}
    int jump_delay;
    bool jumping;
    float original_x;
    float original_y;
    void start_jump();
};

#endif