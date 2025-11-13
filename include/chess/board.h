#pragma once

#include "chess/types.h"

#include <array>
#include <optional>
#include <string>

namespace chess {

class Board {
public:
    Board();

    static Board initialSetup();

    [[nodiscard]] const std::optional<Piece>& at(const Position& pos) const noexcept;
    std::optional<Piece>& at(const Position& pos) noexcept;

    void set(const Position& pos, std::optional<Piece> piece) noexcept;
    void movePiece(const Position& from, const Position& to) noexcept;

    [[nodiscard]] bool isEmpty(const Position& pos) const noexcept;
    void clear() noexcept;

    [[nodiscard]] std::string toString() const;

private:
    std::array<std::array<std::optional<Piece>, 8>, 8> squares_{};
};

} // namespace chess

