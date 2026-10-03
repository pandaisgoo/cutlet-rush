#ifndef STUDENTBICYCLE_H_INCLUDED
#define STUDENTBICYCLE_H_INCLUDED

#include "Student.h"

// fixed settings: StudentNorm attributes
class StudentBicycle : public Student
{
public:
	StudentBicycle(const std::vector<Point> &path) : Student{path, StudentType::BICYCLE, 4} {
		HP = 20;        //學生血量
		v = 70;
		old_v = v;

		bitmap_img_ids.emplace_back(std::vector<int>({0, 1, 2})); 
		bitmap_switch_freq = 20;
	}
};

#endif