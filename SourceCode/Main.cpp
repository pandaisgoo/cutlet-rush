#define ALLEGRO_NO_MAGIC_MAIN
#include "Game.h"
#include <iostream>
#include <string>

int real_main(int argc, char **argv) {
	bool testMode = false;
	if(argc > 1) {
		if(std::string(argv[1]) == "--test") {
			testMode = true;
		}
	}
	Game *game = new Game(testMode);
	if(testMode) {
		delete game;
		std::cout << "Test mode completed.\n";
		return 0;
	}
	game->execute();
	delete game;
	return 0;
}

int main(int argc, char **argv) {
	return al_run_main(argc, argv, real_main);
}
