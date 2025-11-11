#pragma once

#include "Core.h"
#include "definition.h"
#include <optional>


// Create a new thread for the IA 
class Ai {

public:
	Ai(Core* corePtr);
	
	struct Move {
		Vec2 from;
		Vec2 to;
	};

	std::optional<Move> findBestMove(const Core& rootBoard, SIDE sideToMove);

private:
	Core* core;

	const uint8_t maxdepth = 6;

	// helpers
	std::vector<Move> generateAllMoves(const Core& board, SIDE side) const;
	int evaluate(const Core& board) const;
	int pieceValue(PIECE p) const;

	// negamax with alpha-beta
	int negamax(Core board, int depth, SIDE side, int alpha, int beta) const;
};