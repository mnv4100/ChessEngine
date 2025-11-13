#pragma once

#include "chess/board.h"
#include "chess/types.h"

#include <optional>
#include <vector>

namespace chess {

struct CastlingRights {
    bool whiteKingSide{true};
    bool whiteQueenSide{true};
    bool blackKingSide{true};
    bool blackQueenSide{true};
};

struct GameState {
    Board board{};
    Color sideToMove{Color::White};
    CastlingRights castling{};
    std::optional<Position> enPassantTarget{};
    std::uint16_t halfmoveClock{0};
    std::uint16_t fullmoveNumber{1};
};

class Game {
public:
    Game();

    [[nodiscard]] const GameState& state() const noexcept { return state_; }

    [[nodiscard]] std::vector<CategorisedMove> legalMoves() const;
    bool tryMove(const Move& move, std::optional<PieceType> promotion = std::nullopt);

    [[nodiscard]] bool inCheck(Color color) const;
    [[nodiscard]] bool isCheckmate() const;
    [[nodiscard]] bool isStalemate() const;

    void reset();

private:
    GameState state_{};

    [[nodiscard]] bool isSquareAttacked(const GameState& state, const Position& square, Color byColor) const;
    [[nodiscard]] std::vector<CategorisedMove> pseudoLegalMoves(const GameState& state) const;
    [[nodiscard]] GameState applyMove(const GameState& state, const CategorisedMove& move) const;
};

} // namespace chess

