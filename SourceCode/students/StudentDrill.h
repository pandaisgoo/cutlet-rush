#ifndef STUDENTDRILL_H_INCLUDED
#define STUDENTDRILL_H_INCLUDED

#include "Student.h"

// fixed settings: StudentDrill attributes
class StudentDrill : public Student
{
public:
    StudentDrill(const std::vector<Point> &path) 
        : Student{path, StudentType::DRILL, 6} 
    {
        HP = 50;
        v = 30;
        old_v = v;

        bitmap_img_ids.emplace_back(std::vector<int>({0, 1}));
        bitmap_switch_freq = 60;
        bitmap_switch_counter = 60;

        drilling = false;
        drill_cooldown = 0;

        last_x_before_drill = 0;
        last_y_before_drill = 0;

        drill_delay_after_frame1 = 30; 
    }

    bool drilling;          // 是否正在鑽地
    int drill_cooldown;     // 鑽地期間不更新、不畫

    float last_x_before_drill;  // 鑽地前的 X
    float last_y_before_drill;  // 鑽地前的 Y

    int drill_delay_after_frame1;   // 1.png 顯示後等待多少 frame 才消失
};


#endif