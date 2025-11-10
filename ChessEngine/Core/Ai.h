#pragma once

#include "Core.h"
#include "definition.h"
#include <optional>


class Ai {

public:
	Ai(Core* corePtr, SIDE side);
	
	struct Move {
		Vec2 from;
		Vec2 to;
	};

	std::optional<Move> findBestMove(const Core& rootBoard, SIDE sideToMove);

	

private:
	Core* core;
	SIDE side;

	const uint8_t maxdepth = 3;

	// helpers
	std::vector<Move> generateAllMoves(const Core& board, SIDE side) const;
	int evaluate(const Core& board) const;
	int pieceValue(PIECE p) const;

	// negamax with alpha-beta
	int negamax(Core board, int depth, SIDE side, int alpha, int beta) const;
};