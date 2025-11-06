#pragma once
#include <cstdint>

struct Vec2 {
	uint8_t x;
	uint8_t y;

	/*explicit Vec2(const uint8_t x = 0, const uint8_t y = 0) : x(x), y(y) {}*/
};

enum class PIECE : uint8_t {
	King = 0,
	Queen = 1,
	Rook = 4,
	Bishop = 2,
	Knight = 3,
	Pion = 5
};

enum class SIDE : uint8_t {
	WHITE_SIDE = 0,
	BLACK_SIDE = 1
};

union BoardCell {
	uint8_t raw;
	struct {
		uint8_t piece : 3;  // 0�5
		uint8_t side : 1;  // 0�1
		uint8_t fill : 1;  // 0 = empty, 1 = occupied
		uint8_t pad : 3;  // unused (keeps size 1 byte)
	};
};
static_assert(sizeof(BoardCell) == 1, "BoardCell size must be 1 byte");

// Back rank for white (row 0)
constexpr PIECE white_back_rank[8] = {
	PIECE::Rook, PIECE::Knight, PIECE::Bishop, PIECE::Queen,
	PIECE::King, PIECE::Bishop, PIECE::Knight, PIECE::Rook
};

// Pawn row for white (row 1)
constexpr PIECE white_pawns[8] = {
	PIECE::Pion, PIECE::Pion, PIECE::Pion, PIECE::Pion,
	PIECE::Pion, PIECE::Pion, PIECE::Pion, PIECE::Pion
};

// Back rank for black (row 7)
constexpr PIECE black_back_rank[8] = {
	PIECE::Rook, PIECE::Knight, PIECE::Bishop, PIECE::Queen,
	PIECE::King, PIECE::Bishop, PIECE::Knight, PIECE::Rook
};

// Pawn row for black (row 6)
constexpr PIECE black_pawns[8] = {
	PIECE::Pion, PIECE::Pion, PIECE::Pion, PIECE::Pion,
	PIECE::Pion, PIECE::Pion, PIECE::Pion, PIECE::Pion
};