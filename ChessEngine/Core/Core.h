//
// Created by Pc on 05/11/2025.
//

#ifndef CHESSENGINEPROJECT_CORE_H
#define CHESSENGINEPROJECT_CORE_H

#include <cstdint>
#include <array>
#include <map>
#include "definition.h"

/*
	The Controller send a request with a selected piece and a target position
	Check if the move is legal
	if legal we move the piece
		else we return an error

	The Controller send a request for possible moves for a selected piece
	Check all the legal moove for this piece
	return a list of possible moves
*/


class Core {

public:
	explicit Core();
	~Core() {};

	void debugDisplayChessBoard() const;
	bool isMoveLegal(const Vec2& from, const Vec2& to) const;
	bool isPathClear(const Vec2& from, const Vec2& to) const;
	bool movePiece(const Vec2& from, const Vec2& to);

	[[nodiscard]] inline const BoardCell& At(const Vec2& pos) const { return chessBoard[pos.y][pos.x]; }
	[[nodiscard]] inline BoardCell& At(const Vec2& pos) { return chessBoard[pos.y][pos.x]; }

private:
	void fillChessBoard();
	[[nodiscard]] inline bool isMoveInBounds(const Vec2& cell) const;

	inline BoardCell makeCell(PIECE p, SIDE s, bool occupied);
	std::map<SIDE, std::map<PIECE, uint8_t>> takenPiecesCount;
	BoardCell chessBoard[8][8]{};
};


#endif //CHESSENGINEPROJECT_CORE_H