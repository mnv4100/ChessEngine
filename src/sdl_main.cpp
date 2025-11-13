#include "chess/game.h"
#include "chess/sdl_ui.h"

#include <exception>
#include <iostream>

int main() {
    try {
        chess::Game game;
        chess::SdlUI ui(std::move(game));
        ui.run();
    } catch (const std::exception& ex) {
        std::cerr << "Fatal error: " << ex.what() << '\n';
        return 1;
    }
    return 0;
}

