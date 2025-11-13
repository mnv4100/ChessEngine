#include "chess/board.h"

#include <cctype>
#include <sstream>

namespace chess {

namespace {

std::optional<Piece> makePiece(PieceType type, Color color) {
    return Piece{type, color};
}

} // namespace

Board::Board() {
    clear();
}

Board Board::initialSetup() {
    Board board;
    board.clear();

    const auto placeBackRank = [&board](Color color, std::uint8_t rank) {
        std::array<PieceType, 8> order{PieceType::Rook, PieceType::Knight, PieceType::Bishop,
                                       PieceType::Queen, PieceType::King, PieceType::Bishop,
                                       PieceType::Knight, PieceType::Rook};
        for (std::size_t file = 0; file < order.size(); ++file) {
            board.set(Position{static_cast<std::uint8_t>(file), rank}, makePiece(order[file], color));
        }
    };

    for (std::uint8_t file = 0; file < 8; ++file) {
        board.set(Position{file, 1}, makePiece(PieceType::Pawn, Color::Black));
        board.set(Position{file, 6}, makePiece(PieceType::Pawn, Color::White));
    }

    placeBackRank(Color::Black, 0);
    placeBackRank(Color::White, 7);

    return board;
}

const std::optional<Piece>& Board::at(const Position& pos) const noexcept {
    return squares_[pos.rank][pos.file];
}

std::optional<Piece>& Board::at(const Position& pos) noexcept {
    return squares_[pos.rank][pos.file];
}

void Board::set(const Position& pos, std::optional<Piece> piece) noexcept {
    squares_[pos.rank][pos.file] = piece;
}

void Board::movePiece(const Position& from, const Position& to) noexcept {
    squares_[to.rank][to.file] = squares_[from.rank][from.file];
    squares_[from.rank][from.file].reset();
}

bool Board::isEmpty(const Position& pos) const noexcept {
    return !squares_[pos.rank][pos.file].has_value();
}

void Board::clear() noexcept {
    for (auto& rank : squares_) {
        for (auto& cell : rank) {
            cell.reset();
        }
    }
}

std::string Board::toString() const {
    std::ostringstream oss;
    for (std::uint8_t rank = 0; rank < 8; ++rank) {
        oss << static_cast<int>(8 - rank) << " ";
        for (std::uint8_t file = 0; file < 8; ++file) {
            const auto& cell = squares_[rank][file];
            if (!cell) {
                oss << ". ";
                continue;
            }
            const auto index = static_cast<std::size_t>(cell->type);
            char letter = kPieceToChar[index];
            if (cell->color == Color::Black) {
                letter = static_cast<char>(std::tolower(letter));
            }
            oss << letter << ' ';
        }
        oss << '\n';
    }
    oss << "  a b c d e f g h\n";
    return oss.str();
}

} // namespace chess

