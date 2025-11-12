#include "Ai.h"
#include <algorithm>
#include <cstring>

static constexpr int INF = 1000000000;

// Piece value lookup table (cache-friendly)
static constexpr int PIECE_VALUES[6] = {
    100,   // Pion
    320,   // Knight
    330,   // Bishop
    500,   // Rook
    900,   // Queen
    20000  // King
};

Ai::Ai(Core* corePtr)
    : core(corePtr)
{
}

// Optimized: reuse allocated vector
void Ai::generateAllMovesInto(const Core& board, SIDE side, std::vector<Move>& moves) const {
    moves.clear();
    const uint8_t sideValue = static_cast<uint8_t>(side);

    // Single loop, minimal branches
    for (const Vec2& from : board.filledCell) {
        const BoardCell& c = board.At(from);
        if (c.fill == 1 && c.side == sideValue) {
            const auto& tos = board.getPossibleMoves(from);
            const size_t oldSize = moves.size();
            const size_t newSize = oldSize + tos.size();

            // Batch resize once
            moves.resize(newSize);

            // Direct memory copy for Move structs
            Move* dest = moves.data() + oldSize;
            for (size_t i = 0; i < tos.size(); ++i) {
                dest[i] = Move{ from, tos[i] };
            }
        }
    }
}

std::vector<Ai::Move> Ai::generateAllMoves(const Core& board, SIDE side) const {
    std::vector<Move> moves;
    moves.reserve(40);
    generateAllMovesInto(board, side, moves);
    return moves;
}

// Inline-friendly piece value lookup
inline int Ai::pieceValue(PIECE p) const {
    return PIECE_VALUES[static_cast<int>(p)];
}

// Optimized evaluation with minimal branching
int Ai::evaluate(const Core& board) const {
    int score = 0;
    const uint8_t whiteSide = static_cast<uint8_t>(SIDE::WHITE_SIDE);

    for (const Vec2& pos : board.filledCell) {
        const BoardCell& cell = board.At(pos);
        if (cell.fill == 0) continue;

        const int value = PIECE_VALUES[cell.piece];
        // Branchless: (side == white) ? +value : -value
        score += value * (1 - 2 * (cell.side != whiteSide));
    }
    return score;
}

// Key optimization: move ordering without sorting
inline int Ai::scoreMoveForOrdering(const Core& board, const Move& m) const {
    const BoardCell& target = board.At(m.to);
    if (target.fill == 0) return 0;

    // MVV-LVA: Most Valuable Victim - Least Valuable Attacker
    const BoardCell& attacker = board.At(m.from);
    return PIECE_VALUES[target.piece] * 10 - PIECE_VALUES[attacker.piece];
}

int Ai::negamax(Core board, int depth, SIDE side, int alpha, int beta) const {
    if (depth == 0) {
        int val = evaluate(board);
        return (side == SIDE::WHITE_SIDE) ? val : -val;
    }

    // Thread-local move buffer (zero allocation in recursion)
    static thread_local std::vector<Move> moves;
    static thread_local std::vector<int> moveScores;

    generateAllMovesInto(board, side, moves);

    if (moves.empty()) {
        int val = evaluate(board);
        return (side == SIDE::WHITE_SIDE) ? val : -val;
    }

    const int moveCount = static_cast<int>(moves.size());
    int best = -INF;
    const SIDE opp = (side == SIDE::WHITE_SIDE) ? SIDE::BLACK_SIDE : SIDE::WHITE_SIDE;

    // Fast move ordering: score captures, then partial sort only what we need
    if (moveCount > 1) {
        moveScores.resize(moveCount);
        for (int i = 0; i < moveCount; ++i) {
            moveScores[i] = scoreMoveForOrdering(board, moves[i]);
        }

        // Partial sort: only ensure best moves are first (insertion sort for small lists)
        if (moveCount <= 8) {
            for (int i = 1; i < moveCount; ++i) {
                int j = i;
                while (j > 0 && moveScores[j] > moveScores[j-1]) {
                    std::swap(moveScores[j], moveScores[j-1]);
                    std::swap(moves[j], moves[j-1]);
                    --j;
                }
            }
        } else {
            // For larger move lists, only sort first N moves
            constexpr int SORT_COUNT = 8;
            for (int i = 0; i < std::min(SORT_COUNT, moveCount); ++i) {
                int maxIdx = i;
                for (int j = i + 1; j < moveCount; ++j) {
                    if (moveScores[j] > moveScores[maxIdx]) {
                        maxIdx = j;
                    }
                }
                if (maxIdx != i) {
                    std::swap(moveScores[i], moveScores[maxIdx]);
                    std::swap(moves[i], moves[maxIdx]);
                }
            }
        }
    }

    // Search loop
    for (int i = 0; i < moveCount; ++i) {
        const Move& m = moves[i];
        Core tmp = board;
        if (!tmp.movePiece(m.from, m.to)) continue;

        int val = -negamax(tmp, depth - 1, opp, -beta, -alpha);

        if (val > best) {
            best = val;
            if (val > alpha) {
                alpha = val;
                if (alpha >= beta) break; // Beta cutoff
            }
        }
    }

    return best;
}

std::optional<Ai::Move> Ai::findBestMove(const Core& rootBoard, SIDE sideToMove) {
    std::vector<Move> moves;
    moves.reserve(40);
    generateAllMovesInto(rootBoard, sideToMove, moves);

    if (moves.empty()) return std::nullopt;

    int bestVal = -INF;
    std::optional<Move> bestMove = std::nullopt;
    const SIDE opp = (sideToMove == SIDE::WHITE_SIDE) ? SIDE::BLACK_SIDE : SIDE::WHITE_SIDE;

    // Root move ordering
    std::vector<int> moveScores(moves.size());
    for (size_t i = 0; i < moves.size(); ++i) {
        moveScores[i] = scoreMoveForOrdering(rootBoard, moves[i]);
    }

    // Sort root moves by capture value
    for (size_t i = 0; i < moves.size(); ++i) {
        for (size_t j = i + 1; j < moves.size(); ++j) {
            if (moveScores[j] > moveScores[i]) {
                std::swap(moveScores[i], moveScores[j]);
                std::swap(moves[i], moves[j]);
            }
        }
    }

    for (const auto& m : moves) {
        Core tmp = rootBoard;
        if (!tmp.movePiece(m.from, m.to)) continue;

        int val = -negamax(tmp, maxdepth - 1, opp, -INF, INF);

        if (val > bestVal) {
            bestVal = val;
            bestMove = m;
        }
    }

    return bestMove;
}