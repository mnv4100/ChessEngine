#include "chess/console_ui.h"
#include "chess/game.h"

#include <iostream>

int main() {
    chess::Game game;
    chess::ConsoleUI ui(std::move(game), std::cin, std::cout);
    ui.run();
    return 0;
}

