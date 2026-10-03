#ifndef OPERATIONCENTER_H_INCLUDED
#define OPERATIONCENTER_H_INCLUDED

/**
 * @brief Class that defines functions for all object operations.
 * @details Object self-update, draw, and object-to-object interact functions are defined here.
 */
class OperationCenter
{
public:
	static OperationCenter *get_instance() {
		static OperationCenter OC;
		return &OC;
	}
	/**
	 * @brief Highest level update function.
	 * @details Calls all other update functions.
	 */
	void update();
	/**
	 * @brief Highest level draw function.
	 * @details Calls all other draw functions.
	 */
	void draw();
private:
	OperationCenter() {}
private:
	void _update_student();
	void _update_robot();
	void _update_beam();
	void _update_special();
	void _update_special_beam();
	void _update_special_robot();
	void _update_student_beam();
	void _update_student_robot();
private:
	void _draw_student();
	void _draw_robot();
	void _draw_beam();
	void _draw_special();
};

#endif
