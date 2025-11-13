#include "chess/console_ui.h"

#include "chess/notation.h"

#include <algorithm>
#include <cctype>
#include <iostream>

namespace chess {

namespace {

std::string trim(std::string text) {
    const auto isSpace = [](unsigned char c) { return std::isspace(c) != 0; };
    text.erase(text.begin(), std::find_if(text.begin(), text.end(), [&](unsigned char c) { return !isSpace(c); }));
    text.erase(std::find_if(text.rbegin(), text.rend(), [&](unsigned char c) { return !isSpace(c); }).base(), text.end());
    return text;
}

} // namespace

ConsoleUI::ConsoleUI(Game game, std::istream& in, std::ostream& out)
    : game_(std::move(game)), in_(in), out_(out) {}

void ConsoleUI::printBoard() const {
    out_ << "\n" << game_.state().board.toString() << "\n";
}

void ConsoleUI::printStatus() const {
    if (game_.isCheckmate()) {
        out_ << "Checkmate! " << to_string(opposite(game_.state().sideToMove)) << " wins." << '\n';
        return;
    }
    if (game_.isStalemate()) {
        out_ << "Stalemate. The game is drawn." << '\n';
        return;
    }

    out_ << "It is " << to_string(game_.state().sideToMove) << " to move.";
    if (game_.inCheck(game_.state().sideToMove)) {
        out_ << " (check)";
    }
    out_ << "\n";
}

void ConsoleUI::run() {
    out_ << "Welcome to the ChessEngine console interface." << '\n';
    out_ << "Enter moves in long algebraic notation (e2e4, g7g8q for promotion)." << '\n';
    out_ << "Type 'help' for available commands." << '\n';

    std::string line;
    while (true) {
        printBoard();
        printStatus();
        out_ << "> " << std::flush;
        if (!std::getline(in_, line)) {
            out_ << "Input stream closed. Exiting." << '\n';
            break;
        }

        line = trim(line);
        if (line.empty()) {
            continue;
        }

        if (line == "quit" || line == "exit") {
            out_ << "Goodbye!" << '\n';
            break;
        }
        if (line == "help") {
            out_ << "Commands:" << '\n';
            out_ << "  quit/exit  - leave the program" << '\n';
            out_ << "  reset      - restart from the initial position" << '\n';
            out_ << "  fen        - print the board in a simple ASCII format" << '\n';
            out_ << "Or enter a move such as e2e4 or g7g8q." << '\n';
            continue;
        }
        if (line == "reset") {
            game_.reset();
            out_ << "Game reset." << '\n';
            continue;
        }
        if (line == "fen") {
            out_ << game_.state().board.toString();
            continue;
        }

        const auto parsed = parseCoordinateMove(line);
        if (!parsed) {
            out_ << "Could not parse move: '" << line << "'." << '\n';
            continue;
        }

        if (game_.tryMove(*parsed, parsed->promotion)) {
            if (game_.isCheckmate()) {
                printBoard();
                printStatus();
                break;
            }
            if (game_.isStalemate()) {
                printBoard();
                printStatus();
                break;
            }
        } else {
            out_ << "Illegal move." << '\n';
        }
    }
}

} // namespace chess

