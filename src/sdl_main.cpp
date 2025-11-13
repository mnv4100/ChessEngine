#include "chess/game.h"
#include "chess/sdl_ui.h"

#include <exception>
#include <iostream>

#include <SDL3/SDL_main.h>

int SDL_main(int argc, char* argv[]) {
	try {
		chess::Game game;
		chess::SdlUI ui(std::move(game));
		ui.run();
	}
	catch (const std::exception& ex) {
		std::cerr << "Fatal error: " << ex.what() << '\n';
		return 1;
	}
	return 0;
}



