#include "Ai.h"
#include <limits>
#include <algorithm>
#include <array>
#include <cstdint>
#include <immintrin.h>
#include <bit>    // std::assume_aligned (C++20)

static constexpr int INF = 1000000000;


// take the woll board cache it 
// update the cash when a move is made
// use the cache to evaluate positions faster



Ai::Ai(Core* corePtr, SIDE side)
	: core(corePtr), side(side)
{

}



inline void process_board(const BoardCell(&b)[64]) {
#if defined(__AVX512F__)
    const auto* p = std::assume_aligned<64>(reinterpret_cast<const std::uint8_t*>(b));
    __m512i v = _mm512_load_si512(p);         // one 64B load
    // ... operate on v ...
    // _mm512_store_si512(...) to write back if needed
#elif defined(__AVX2__)
    const auto* p = std::assume_aligned<32>(reinterpret_cast<const std::uint8_t*>(b));
    const __m256i lo = _mm256_load_si256(reinterpret_cast<const __m256i*>(p + 0));
    const __m256i hi = _mm256_load_si256(reinterpret_cast<const __m256i*>(p + 32));
    // ... operate on lo/hi ...
    // _mm256_store_si256(...) to write back
#else
    // Portable: many compilers will auto-vectorize this with -O3 -march=native
    unsigned acc = 0;
    for (unsigned i = 0; i < 64; ++i) acc += b[i].raw;
    (void)acc;
#endif
}


std::vector<Ai::Move> Ai::generateAllMoves(const Core& board, SIDE side) const {
    std::vector<Move> out;
    for (uint8_t y = 0; y < 8; ++y) {
        for (uint8_t x = 0; x < 8; ++x) {
            Vec2 from{ x, y };
            const BoardCell& c = board.At(from);
            if (c.fill == 1 && c.side == static_cast<uint8_t>(side)) {
                std::vector<Vec2> tos = board.getPossibleMoves(from);
                for (const auto& t : tos) {
                    out.push_back(Move{ from, t });
                }
            }
        }
    }
    return out;
}


int Ai::pieceValue(PIECE p) const {
    switch (p) {
    case PIECE::Pion:   return 100;
    case PIECE::Knight: return 320;
    case PIECE::Bishop: return 330;
    case PIECE::Rook:   return 500;
    case PIECE::Queen:  return 900;
    case PIECE::King:   return 20000;
    }
    return 0;
}


int Ai::evaluate(const Core& board) const {
    int score = 0;
    for (uint8_t y = 0; y < 8; ++y) {
        for (uint8_t x = 0; x < 8; ++x) {
            Vec2 pos{ x, y };
            const BoardCell& cell = board.At(pos);
            if (cell.fill == 0) continue;
            PIECE p = static_cast<PIECE>(cell.piece);
            int val = pieceValue(p);
            if (cell.side == static_cast<uint8_t>(SIDE::WHITE_SIDE)) score += val;
            else score -= val;
        }
    }
    return score; // positive = white is better
}


int Ai::negamax(Core board, int depth, SIDE side, int alpha, int beta) const {
    if (depth == 0) {
        int val = evaluate(board);
        return (side == SIDE::WHITE_SIDE) ? val : -val;
    }

    auto moves = generateAllMoves(board, side);
    if (moves.empty()) {
        // no moves: return evaluation (checkmate/stalemate will be expressed in eval)
        int val = evaluate(board);
        return (side == SIDE::WHITE_SIDE) ? val : -val;
    }

    int best = -INF;
    SIDE opp = (side == SIDE::WHITE_SIDE) ? SIDE::BLACK_SIDE : SIDE::WHITE_SIDE;

    // simple move ordering: try captures first (cheap heuristic)
    std::stable_sort(moves.begin(), moves.end(), [&](const Move& a, const Move& b) {
        const BoardCell ca = board.At(a.to);
        const BoardCell cb = board.At(b.to);
        return (ca.fill ? 1 : 0) > (cb.fill ? 1 : 0);
        });

    for (const auto& m : moves) {
        Core tmp = board;               // copy
        bool ok = tmp.movePiece(m.from, m.to);
        if (!ok) continue;
        int val = -negamax(tmp, depth - 1, opp, -beta, -alpha);
        best = std::max(best, val);
        alpha = std::max(alpha, val);
        if (alpha >= beta) break; // cutoff
    }
    return best;
}

std::optional<Ai::Move> Ai::findBestMove(const Core& rootBoard, SIDE sideToMove) {
    auto moves = generateAllMoves(rootBoard, sideToMove);
    if (moves.empty()) return std::nullopt;

    int bestVal = -INF;
    std::optional<Move> bestMove = std::nullopt;
    SIDE opp = (sideToMove == SIDE::WHITE_SIDE) ? SIDE::BLACK_SIDE : SIDE::WHITE_SIDE;

    // root loop (no need to copy rootBoard every time; we will copy per move)
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