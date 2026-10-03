#include "DataCenter.h"
#include <cstring>
#include "../Level.h"
#include "../Player.h"
#include "../students/Student.h"
#include "../special/Special.h"
#include "../robots/Robot.h"
#include "../Star.h"

// fixed settings
namespace DataSetting {
	constexpr double FPS = 60;
	constexpr int window_width = 800; 
	constexpr int window_height = 600; 
	constexpr int game_field_length = 600;
}



DataCenter::DataCenter() {
	this->FPS = DataSetting::FPS;
	this->window_width = DataSetting::window_width;
	this->window_height = DataSetting::window_height;
	this->game_field_length = DataSetting::game_field_length;
	memset(key_state, false, sizeof(key_state));
	memset(prev_key_state, false, sizeof(prev_key_state));
	mouse = Point(0, 0);
	memset(mouse_state, false, sizeof(mouse_state));
	memset(prev_mouse_state, false, sizeof(prev_mouse_state));
	player = new Player();
	level = new Level();
}

DataCenter::~DataCenter() {
	delete player;
	delete level;
	for (Student *&s : students) {
		delete s;
	}
	for (Robot *&r : robots) {
		delete r;
	}
	for(Beam *&bb : beams) {
		delete bb;
	}
	for(Special *&ss : specials) {
		delete ss;
	}
	for(Star *&s_s : stars) {
		delete s_s;
	}
}
