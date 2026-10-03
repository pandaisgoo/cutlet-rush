#include "OperationCenter.h"
#include "DataCenter.h"
#include "../Level.h"
#include "../Player.h"
#include "../robots/Robot.h"
#include "../robots/RobotBomb.h"
#include "../robots/Beam.h"
#include "../students/Student.h"
#include "../students/StudentJump.h"
#include "../students/StudentBooo.h"
#include "../special/Special.h"
#include "../special/SpecialTyphoon.h"
#include "../special/SpecialWind.h"

void OperationCenter::update() {
	_update_student();
	_update_robot();
	_update_beam();
	_update_special();
    _update_special_robot();
	_update_special_beam();
	_update_student_beam();
	_update_student_robot();
}


void OperationCenter::_update_student() {
	std::vector<Student*> &students = DataCenter::get_instance()->students;
	for(Student *student : students)
		student->update();
}

void OperationCenter::_update_robot() {
	std::vector<Robot*> &robots = DataCenter::get_instance()->robots;
	for(Robot *robot : robots)
		robot->update();
}

void OperationCenter::_update_beam() {
	std::vector<Beam*> &beams = DataCenter::get_instance()->beams;
	for(Beam *beam : beams)
		beam->update();
	// Detect if a bullet flies too far (exceeds its fly distance limit), which means the bullet lifecycle has ended.
	for(size_t i = 0; i < beams.size(); ++i) {
		if(beams[i]->get_fly_dist() <= 0) {
			delete beams[i];
			beams.erase(beams.begin() + i);
			--i;
		}
	}
}

void OperationCenter::_update_special() {
    auto& specials = DataCenter::get_instance()->specials;
    for (Special* sp : specials)
        sp->update();
}


void OperationCenter::_update_special_beam()
{
    DataCenter* DC = DataCenter::get_instance();

    auto& beams = DC->beams;
    auto& specials = DC->specials;

    for (auto bit = beams.begin(); bit != beams.end(); )
    {
        Beam* beam = *bit;
        bool beam_removed = false;

        // ------------------------------
        // 檢查 Beam 是否打到 Special
        // ------------------------------
        for (auto sit = specials.begin(); sit != specials.end(); )
        {
            Special* sp = *sit;

            // ⭐ 特例：Typhoon 在死亡動畫時不能再被打
            if (sp->get_type() == SpecialType::TYPHOON) {
                SpecialTyphoon* t = static_cast<SpecialTyphoon*>(sp);
                if (t->dying) { 
                    ++sit;
                    continue;
                }
            }
			if (sp->get_type() == SpecialType::WIND) {
                SpecialWind* w = static_cast<SpecialWind*>(sp);
                if (w->dying) { 
                    ++sit;
                    continue;
                }
            }

            if (beam->shape->overlap(*(sp->shape)))
			{
				sp->HP -= beam->get_dmg();

				// ★★★ 特攻學生死亡改成啟動死亡動畫，不刪除
				if (sp->HP <= 0) {
					sp->HP = 0;
					// 由 Special::update() 控制死亡動畫，這裡不要刪除
				}

				delete beam;
				bit = beams.erase(bit);
				beam_removed = true;
				break;
			}
            else {
                ++sit;
            }
        }

        if (!beam_removed)
            ++bit;
    }
}

// void OperationCenter::_update_special_robot()
// {
//     DataCenter* DC = DataCenter::get_instance();
//     auto& specials = DC->specials;
//     auto& robots   = DC->robots;

//     for (Special* sp : specials)
//     {
//         // 死亡動畫中的 special 不要做碰撞
//         if (sp->dying) continue;

//         for (Robot* rb : robots)
//         {
//             if (rb->HP <= 0) continue;

//             // ⭐ 判斷 Special 與 Robot 是否 overlap
//             if (sp->shape->overlap(*(rb->shape)))
//             {
//                 // ====== ★ 行為 A：互相扣血 ======
//                 rb->HP -= 5;       // Robot 扣血（你可以改數值）
//                 sp->HP = 0;       // Special 扣血

//                 // ====== ★ 行為 B：Special 死亡 → 啟動死亡動畫 ======
//                 if (sp->HP <= 0)
//                 {
//                     sp->HP = 0;  // Special.cpp 會自動切死亡動畫
//                 }

//                 // ====== ★ 行為 C：Robot 死亡 → 交給 Game/Level 清掉 ======
//                 if (rb->HP <= 0)
//                 {
//                     // robot->HP = 0 就好，千萬不要 delete！
//                 }

//                 break; // 避免同一 special 同時撞上多台 robot
//             }
//         }
//     }
// }
// void OperationCenter::_update_special_robot()
// {
//     DataCenter* DC = DataCenter::get_instance();

//     for (Robot* robot : DC->robots)
//     {
//         if (robot->HP <= 0) continue;

//         for (Special* sp : DC->specials)
//         {
//             if (sp->HP <= 0 || sp->dying) continue;

//             double ry = robot->shape->center_y();
//             double sy = sp->shape->center_y();

//             if (robot->shape->overlap(*(sp->shape)) &&
//                 std::abs(ry - sy) <= 10)
//             {
//                 // ⭐ Special 停下來
//                 sp->stop();

//                 // ⭐ Special 攻擊 robot
//                 robot->HP -= 5;

//                 // robot 死了 → special 繼續走
//                 if (robot->HP <= 0)
//                     sp->recover();

//                 break; // 下一個 special
//             }
//             else {
//                 // ⭐ 不再重疊 → Special 恢復走路
//                 sp->recover();
//             }
//         }
//     }
// }
void OperationCenter::_update_special_robot()
{
    DataCenter* DC = DataCenter::get_instance();

    for (Special* sp : DC->specials)
    {
        if (sp->HP <= 0 || sp->dying) continue;

        bool blocked = false;   // ⭐ special 是否真的被某台 robot 擋住

        for (Robot* robot : DC->robots)
        {
            if (robot->HP <= 0) continue;

            double ry = robot->shape->center_y();
            double sy = sp->shape->center_y();

            // ⭐ 檢查碰撞
            if (robot->shape->overlap(*(sp->shape)) &&
                std::abs(ry - sy) <= 10)
            {
                // special 停下來
                sp->stop();
                blocked = true;

                // special 攻擊 robot
                robot->HP -= 5;

                // robot 死掉 → special 可以繼續走
                if (robot->HP <= 0)
                    sp->recover();

                break;  // 特效：一台 robot 就夠擋住
            }
        }

        // ⭐ 完整 loop 結束後才決定是否 recover
        if (!blocked)
            sp->recover();
    }
}




void OperationCenter::_update_student_beam()
{
    DataCenter* DC = DataCenter::get_instance();
    auto& beams = DC->beams;
    auto& students = DC->students;

    for (auto bit = beams.begin(); bit != beams.end(); )
    {
        Beam* beam = *bit;
        bool removed = false;

		if (beam->get_type() == RobotType::GENERATOR) {
            ++bit;
            continue;   // ← 直接跳過，不檢查學生
        }

        for (auto sit = students.begin(); sit != students.end(); )
        {
            Student* stu = *sit;
            

            if (beam->shape->overlap(*(stu->shape))) {

                stu->HP -= beam->get_dmg();

                if (beam->get_type() == RobotType::ICE) {
                    stu->chanege_speed();
                }

                // delete student if dead
                if (stu->HP <= 0) {
                    delete stu;
                    sit = students.erase(sit);    // safe erase
                }
                else {
                    ++sit;
                }

                // remove beam
                delete beam;
                bit = beams.erase(bit);
                removed = true;
                break;
            }
            else {
                ++sit;
            }
        }

        if (!removed) {
            ++bit;
        }
    }
}

void OperationCenter::_update_student_robot()
{
    DataCenter* DC = DataCenter::get_instance();
    auto& robots = DC->robots;
    auto& students = DC->students;

    // =================================================
    // BOOO 爆炸 × Robot 碰撞判斷
    // =================================================
    for (Student* stu : DC->students) {

        if (stu->get_type() != StudentType::BOOO)
            continue;

        StudentBooo* b = static_cast<StudentBooo*>(stu);

        // 只在爆炸期間處理
        if (!b->exploded)
            continue;

        // 只傷害一次
        // if (b->has_damaged)
        //     continue;

        for (Robot* r : DC->robots) {

            if (r->HP <= 0)
                continue;

            bool hit = false;

            // ⭐ 用 explode_hitboxes 判斷
            for (Rectangle* hb : b->explode_hitboxes) {
                if (!hb) continue;

                if (hb->overlap(*(r->shape))) {
                    hit = true;
                    break;
                }
            }

            if (hit) {
                r->HP  = 0;            // 💥 爆炸傷害
                // b->has_damaged = true; // ⭐ 防止重複傷害
                //break;                  // 一次爆炸只影響一次
            }
        }
    }


    for (Robot* robot : robots)
    {
        if (robot->HP <= 0) continue;


        for (Student* stu : students)
        {
            if (stu->HP <= 0) continue;
            

            // Y軸檢查
            double robotY = robot->shape->center_y();
            double stuY = stu->shape->center_y();

            if (robot->shape->overlap(*(stu->shape)) && std::abs(robotY - stuY) <= 10)
            {
                if (stu->get_type() == StudentType::JUMP) {
                    StudentJump* j = static_cast<StudentJump*>(stu);
                    j->start_jump(); // ⭐ 碰到 robot 才能跳
                    continue;
                }
            }

            Point sg = DC->level->world_to_block(
                stu->shape->center_x(),
                stu->shape->center_y()
            );
            Point rg = DC->level->world_to_block(
                robot->shape->center_x(),
                robot->shape->center_y()
            );
            if (std::abs(robotY - stuY) > 30) continue;

            // 一般機器人的阻擋邏輯
            //if (robot->shape->overlap(*(stu->shape))) {
            if (sg.x < 0 || rg.x < 0) continue;

            if (sg.x == rg.x && sg.y == rg.y) {
                 stu->stop();
                 robot->HP -= stu->get_dmg();
                 
                 if (robot->HP <= 0) {
                     stu->recover();
                 }
            }
        }
    }
}

void OperationCenter::draw() {
    _draw_robot();
	_draw_student();
	_draw_beam();
	_draw_special();
}


void OperationCenter::_draw_student() {
	std::vector<Student*> &students = DataCenter::get_instance()->students;
	for(Student *student : students)
		student->draw();
}

void OperationCenter::_draw_robot() {
	std::vector<Robot*> &robots = DataCenter::get_instance()->robots;
	for(Robot *robot : robots)
		robot->draw();
}

void OperationCenter::_draw_beam() {
	std::vector<Beam*> &beams = DataCenter::get_instance()->beams;
	for(Beam *beam : beams)
		beam->draw();
}

void OperationCenter::_draw_special() {
    auto& specials = DataCenter::get_instance()->specials;
    for (Special* sp : specials)
        sp->draw();
}

