#pragma once

#include "chess/game.h"

#include <istream>
#include <ostream>
#include <utility>

namespace chess {

class ConsoleUI {
public:
    ConsoleUI(Game game, std::istream& in, std::ostream& out);

    void run();

private:
    void printBoard() const;
    void printStatus() const;

    Game game_;
    std::istream& in_;
    std::ostream& out_;
};

} // namespace chess

