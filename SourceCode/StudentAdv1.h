#ifndef STUDENTADV1_H_INCLUDED
#define STUDENTADV1_H_INCLUDED

#include "Student.h"

// fixed settings: StudentNorm attributes
class StudentAdv1 : public Student
{
public:
	StudentAdv1(const std::vector<Point> &path) : Student{path, StudentType::ADV1, 2} {
		HP = 10;        //學生血量
		v = 80;
		old_v = v;

		bitmap_img_ids.emplace_back(std::vector<int>({0, 1})); 
		bitmap_switch_freq = 20;
	}
};

#endif