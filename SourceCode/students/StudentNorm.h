#ifndef STUDENTNORM_H_INCLUDED
#define STUDENTNORM_H_INCLUDED

#include "Student.h"

// fixed settings: StudentNorm attributes
class StudentNorm : public Student
{
public:
	StudentNorm(const std::vector<Point> &path) : Student{path, StudentType::NORM, 2} {
		HP = 10;        //學生血量
		v = 30;
		old_v = v;

		bitmap_img_ids.emplace_back(std::vector<int>({0, 1})); 
		bitmap_switch_freq = 20;
	}
};

#endif