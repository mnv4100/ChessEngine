//
// Created by Pc on 05/11/2025.
//

#ifndef CHESSENGINEPROJECT_CORE_H
#define CHESSENGINEPROJECT_CORE_H

#include <cstdint>
#include <array>
#include <map>
#include <vector>

#include "definition.h"

class Core {

public:
	explicit Core();
	~Core() = default;

	void debugDisplayChessBoard() const;
	bool isMoveLegal(const Vec2& from, const Vec2& to) const;
	bool isPathClear(const Vec2& from, const Vec2& to) const;
	bool movePiece(const Vec2& from, const Vec2& to);
	std::vector<Vec2> getPossibleMoves(const Vec2& from) const;

	[[nodiscard]] const BoardCell& At(const Vec2& pos) const { return chessBoard[pos.y * 8 + pos.x];  };
	[[nodiscard]] BoardCell& At(const Vec2& pos) { return chessBoard[pos.y * 8 + pos.x]; };

private:
	void fillChessBoard();
	[[nodiscard]] inline bool isMoveInBounds(const Vec2& cell) const;

	constexpr inline BoardCell makeCell(PIECE p, SIDE s, bool occupied) noexcept;
	std::map<SIDE, std::map<PIECE, uint8_t>> takenPiecesCount;
	
	// 1D array to use full one line of cache 64 bits
	alignas(64) BoardCell chessBoard[64]{};
};


#endif //CHESSENGINEPROJECT_CORE_H