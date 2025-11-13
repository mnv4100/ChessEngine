#include "chess/game.h"

#include <algorithm>
#include <array>
#include <stdexcept>

namespace chess {
namespace {

Position findKing(const Board& board, Color color) {
    for (std::uint8_t rank = 0; rank < 8; ++rank) {
        for (std::uint8_t file = 0; file < 8; ++file) {
            Position pos{file, rank};
            const auto& cell = board.at(pos);
            if (cell && cell->type == PieceType::King && cell->color == color) {
                return pos;
            }
        }
    }
    throw std::runtime_error("king not found on board");
}

bool isInitialRookSquare(Color color, const Position& pos, bool kingSide) {
    const std::uint8_t rank = color == Color::White ? 7 : 0;
    const std::uint8_t file = kingSide ? 7 : 0;
    return pos.rank == rank && pos.file == file;
}

void disableCastlingForRook(CastlingRights& castling, Color color, const Position& from) {
    if (color == Color::White) {
        if (isInitialRookSquare(color, from, true)) {
            castling.whiteKingSide = false;
        } else if (isInitialRookSquare(color, from, false)) {
            castling.whiteQueenSide = false;
        }
    } else {
        if (isInitialRookSquare(color, from, true)) {
            castling.blackKingSide = false;
        } else if (isInitialRookSquare(color, from, false)) {
            castling.blackQueenSide = false;
        }
    }
}

void disableCastlingForCapturedRook(CastlingRights& castling, const Position& pos) {
    if (pos.rank == 7 && pos.file == 0) {
        castling.whiteQueenSide = false;
    } else if (pos.rank == 7 && pos.file == 7) {
        castling.whiteKingSide = false;
    } else if (pos.rank == 0 && pos.file == 0) {
        castling.blackQueenSide = false;
    } else if (pos.rank == 0 && pos.file == 7) {
        castling.blackKingSide = false;
    }
}

bool addOffset(Position base, int deltaFile, int deltaRank, Position& out) {
    const int newFile = static_cast<int>(base.file) + deltaFile;
    const int newRank = static_cast<int>(base.rank) + deltaRank;
    if (newFile < 0 || newFile >= 8 || newRank < 0 || newRank >= 8) {
        return false;
    }
    out = Position{static_cast<std::uint8_t>(newFile), static_cast<std::uint8_t>(newRank)};
    return true;
}

} // namespace

Game::Game() {
    reset();
}

void Game::reset() {
    state_.board = Board::initialSetup();
    state_.sideToMove = Color::White;
    state_.castling = {};
    state_.enPassantTarget.reset();
    state_.halfmoveClock = 0;
    state_.fullmoveNumber = 1;
}

bool Game::tryMove(const Move& move, std::optional<PieceType> promotion) {
    const auto legal = legalMoves();
    for (const auto& candidate : legal) {
        if (candidate.move.from != move.from || candidate.move.to != move.to) {
            continue;
        }
        if (candidate.category == MoveCategory::Promotion) {
            auto desired = promotion ? promotion : candidate.promotion;
            if (!desired || !candidate.promotion || desired != candidate.promotion) {
                continue;
            }
        }
        state_ = applyMove(state_, candidate);
        return true;
    }
    return false;
}

std::vector<CategorisedMove> Game::legalMoves() const {
    const auto pseudo = pseudoLegalMoves(state_);
    std::vector<CategorisedMove> legal;
    legal.reserve(pseudo.size());

    const auto enemy = opposite(state_.sideToMove);
    const auto currentKingPos = findKing(state_.board, state_.sideToMove);

    for (const auto& move : pseudo) {
        if (move.category == MoveCategory::KingSideCastle || move.category == MoveCategory::QueenSideCastle) {
            const std::uint8_t rank = state_.sideToMove == Color::White ? 7 : 0;
            const Position fSquare{5, rank};
            const Position dSquare{3, rank};

            if (isSquareAttacked(state_, currentKingPos, enemy)) {
                continue;
            }

            if (move.category == MoveCategory::KingSideCastle) {
                if (isSquareAttacked(state_, fSquare, enemy)) {
                    continue;
                }
            } else {
                if (isSquareAttacked(state_, dSquare, enemy)) {
                    continue;
                }
            }
        }

        auto nextState = applyMove(state_, move);
        const auto kingPos = findKing(nextState.board, state_.sideToMove);
        if (!isSquareAttacked(nextState, kingPos, enemy)) {
            legal.push_back(move);
        }
    }

    return legal;
}

bool Game::inCheck(Color color) const {
    const auto kingPos = findKing(state_.board, color);
    return isSquareAttacked(state_, kingPos, opposite(color));
}

bool Game::isCheckmate() const {
    if (!inCheck(state_.sideToMove)) {
        return false;
    }
    return legalMoves().empty();
}

bool Game::isStalemate() const {
    if (inCheck(state_.sideToMove)) {
        return false;
    }
    return legalMoves().empty();
}

bool Game::isSquareAttacked(const GameState& state, const Position& square, Color byColor) const {
    const auto& board = state.board;

    const int pawnRankOffset = byColor == Color::White ? 1 : -1;
    for (int deltaFile : {-1, 1}) {
        Position candidate{};
        if (addOffset(square, deltaFile, pawnRankOffset, candidate)) {
            const auto& cell = board.at(candidate);
            if (cell && cell->color == byColor && cell->type == PieceType::Pawn) {
                return true;
            }
        }
    }

    const std::array<std::pair<int, int>, 8> knightMoves{{{1, 2}, {2, 1}, {2, -1}, {1, -2}, {-1, 2}, {-2, 1}, {-2, -1}, {-1, -2}}};
    for (const auto& [df, dr] : knightMoves) {
        Position candidate{};
        if (!addOffset(square, df, dr, candidate)) {
            continue;
        }
        const auto& cell = board.at(candidate);
        if (cell && cell->color == byColor && cell->type == PieceType::Knight) {
            return true;
        }
    }

    const std::array<std::pair<int, int>, 4> bishopDirs{{{1, 1}, {1, -1}, {-1, 1}, {-1, -1}}};
    for (const auto& [df, dr] : bishopDirs) {
        Position current = square;
        while (addOffset(current, df, dr, current)) {
            const auto& cell = board.at(current);
            if (!cell) {
                continue;
            }
            if (cell->color == byColor && (cell->type == PieceType::Bishop || cell->type == PieceType::Queen)) {
                return true;
            }
            break;
        }
    }

    const std::array<std::pair<int, int>, 4> rookDirs{{{1, 0}, {-1, 0}, {0, 1}, {0, -1}}};
    for (const auto& [df, dr] : rookDirs) {
        Position current = square;
        while (addOffset(current, df, dr, current)) {
            const auto& cell = board.at(current);
            if (!cell) {
                continue;
            }
            if (cell->color == byColor && (cell->type == PieceType::Rook || cell->type == PieceType::Queen)) {
                return true;
            }
            break;
        }
    }

    for (const auto& [df, dr] : bishopDirs) {
        Position candidate{};
        if (addOffset(square, df, dr, candidate)) {
            const auto& cell = board.at(candidate);
            if (cell && cell->color == byColor && cell->type == PieceType::King) {
                return true;
            }
        }
    }
    for (const auto& [df, dr] : rookDirs) {
        Position candidate{};
        if (addOffset(square, df, dr, candidate)) {
            const auto& cell = board.at(candidate);
            if (cell && cell->color == byColor && cell->type == PieceType::King) {
                return true;
            }
        }
    }

    return false;
}

std::vector<CategorisedMove> Game::pseudoLegalMoves(const GameState& state) const {
    std::vector<CategorisedMove> moves;
    const auto& board = state.board;
    const auto side = state.sideToMove;

    const auto pushMove = [&moves](const Move& move, MoveCategory category, std::optional<PieceType> promotion = std::nullopt) {
        CategorisedMove entry{move, category, promotion};
        if (promotion) {
            entry.move.promotion = promotion;
        }
        moves.push_back(entry);
    };

    const auto addPawnMoves = [&](const Position& pos, const Piece& piece) {
        const int direction = piece.color == Color::White ? -1 : 1;
        const int startRank = piece.color == Color::White ? 6 : 1;
        Position forward{};
        if (addOffset(pos, 0, direction, forward) && board.isEmpty(forward)) {
            if ((piece.color == Color::White && forward.rank == 0) ||
                (piece.color == Color::Black && forward.rank == 7)) {
                for (PieceType promotion : {PieceType::Queen, PieceType::Rook, PieceType::Bishop, PieceType::Knight}) {
                    pushMove(Move{pos, forward, promotion}, MoveCategory::Promotion, promotion);
                }
            } else {
                pushMove(Move{pos, forward, std::nullopt}, MoveCategory::Quiet);
            }

            Position doubleForward{};
            if (pos.rank == startRank && addOffset(pos, 0, 2 * direction, doubleForward) && board.isEmpty(doubleForward)) {
                pushMove(Move{pos, doubleForward, std::nullopt}, MoveCategory::DoublePawnPush);
            }
        }

        for (int deltaFile : {-1, 1}) {
            Position capture{};
            if (!addOffset(pos, deltaFile, direction, capture)) {
                continue;
            }
            const auto& target = board.at(capture);
            if (target && target->color != piece.color) {
                if ((piece.color == Color::White && capture.rank == 0) ||
                    (piece.color == Color::Black && capture.rank == 7)) {
                    for (PieceType promotion : {PieceType::Queen, PieceType::Rook, PieceType::Bishop, PieceType::Knight}) {
                        pushMove(Move{pos, capture, promotion}, MoveCategory::Promotion, promotion);
                    }
                } else {
                    pushMove(Move{pos, capture, std::nullopt}, MoveCategory::Capture);
                }
            } else if (state.enPassantTarget && capture == *state.enPassantTarget) {
                pushMove(Move{pos, capture, std::nullopt}, MoveCategory::EnPassant);
            }
        }
    };

    const auto addKnightMoves = [&](const Position& pos, const Piece& piece) {
        const std::array<std::pair<int, int>, 8> offsets{{{1, 2}, {2, 1}, {2, -1}, {1, -2}, {-1, 2}, {-2, 1}, {-2, -1}, {-1, -2}}};
        for (const auto& [df, dr] : offsets) {
            Position dst{};
            if (!addOffset(pos, df, dr, dst)) {
                continue;
            }
            const auto& cell = board.at(dst);
            if (!cell) {
                pushMove(Move{pos, dst, std::nullopt}, MoveCategory::Quiet);
            } else if (cell->color != piece.color) {
                pushMove(Move{pos, dst, std::nullopt}, MoveCategory::Capture);
            }
        }
    };

    const auto addSlidingMoves = [&](const Position& pos, const Piece& piece, const std::array<std::pair<int, int>, 4>& directions) {
        for (const auto& [df, dr] : directions) {
            Position current = pos;
            while (addOffset(current, df, dr, current)) {
                const auto& cell = board.at(current);
                if (!cell) {
                    pushMove(Move{pos, current, std::nullopt}, MoveCategory::Quiet);
                    continue;
                }
                if (cell->color != piece.color) {
                    pushMove(Move{pos, current, std::nullopt}, MoveCategory::Capture);
                }
                break;
            }
        }
    };

    const auto addKingMoves = [&](const Position& pos, const Piece& piece) {
        for (int df = -1; df <= 1; ++df) {
            for (int dr = -1; dr <= 1; ++dr) {
                if (df == 0 && dr == 0) {
                    continue;
                }
                Position dst{};
                if (!addOffset(pos, df, dr, dst)) {
                    continue;
                }
                const auto& cell = board.at(dst);
                if (!cell) {
                    pushMove(Move{pos, dst, std::nullopt}, MoveCategory::Quiet);
                } else if (cell->color != piece.color) {
                    pushMove(Move{pos, dst, std::nullopt}, MoveCategory::Capture);
                }
            }
        }

        const bool canCastleKingSide = piece.color == Color::White ? state.castling.whiteKingSide : state.castling.blackKingSide;
        const bool canCastleQueenSide = piece.color == Color::White ? state.castling.whiteQueenSide : state.castling.blackQueenSide;
        const std::uint8_t rank = piece.color == Color::White ? 7 : 0;

        if (canCastleKingSide) {
            const Position rookPos{7, rank};
            const auto& rookCell = board.at(rookPos);
            if (rookCell && rookCell->color == piece.color && rookCell->type == PieceType::Rook) {
                if (board.isEmpty(Position{5, rank}) && board.isEmpty(Position{6, rank})) {
                    pushMove(Move{pos, Position{6, rank}, std::nullopt}, MoveCategory::KingSideCastle);
                }
            }
        }
        if (canCastleQueenSide) {
            const Position rookPos{0, rank};
            const auto& rookCell = board.at(rookPos);
            if (rookCell && rookCell->color == piece.color && rookCell->type == PieceType::Rook) {
                if (board.isEmpty(Position{1, rank}) && board.isEmpty(Position{2, rank}) && board.isEmpty(Position{3, rank})) {
                    pushMove(Move{pos, Position{2, rank}, std::nullopt}, MoveCategory::QueenSideCastle);
                }
            }
        }
    };

    for (std::uint8_t rank = 0; rank < 8; ++rank) {
        for (std::uint8_t file = 0; file < 8; ++file) {
            Position pos{file, rank};
            const auto& cell = board.at(pos);
            if (!cell || cell->color != side) {
                continue;
            }

            switch (cell->type) {
            case PieceType::Pawn:
                addPawnMoves(pos, *cell);
                break;
            case PieceType::Knight:
                addKnightMoves(pos, *cell);
                break;
            case PieceType::Bishop:
                addSlidingMoves(pos, *cell, {{{1, 1}, {1, -1}, {-1, 1}, {-1, -1}}});
                break;
            case PieceType::Rook:
                addSlidingMoves(pos, *cell, {{{1, 0}, {-1, 0}, {0, 1}, {0, -1}}});
                break;
            case PieceType::Queen:
                addSlidingMoves(pos, *cell, {{{1, 1}, {1, -1}, {-1, 1}, {-1, -1}}});
                addSlidingMoves(pos, *cell, {{{1, 0}, {-1, 0}, {0, 1}, {0, -1}}});
                break;
            case PieceType::King:
                addKingMoves(pos, *cell);
                break;
            }
        }
    }

    return moves;
}

GameState Game::applyMove(const GameState& state, const CategorisedMove& move) const {
    GameState next = state;
    auto& board = next.board;
    const auto movingPiece = board.at(move.move.from);
    if (!movingPiece) {
        throw std::runtime_error("attempted to move a non-existent piece");
    }

    next.enPassantTarget.reset();

    // Handle captures impacting castling rights.
    const auto& target = board.at(move.move.to);
    if (target && target->color != movingPiece->color) {
        disableCastlingForCapturedRook(next.castling, move.move.to);
    }

    board.movePiece(move.move.from, move.move.to);

    if (move.category == MoveCategory::Promotion && move.promotion) {
        board.at(move.move.to) = Piece{*move.promotion, movingPiece->color};
    }

    if (move.category == MoveCategory::EnPassant) {
        Position captured{move.move.to.file, static_cast<std::uint8_t>(move.move.from.rank)};
        board.at(captured).reset();
    }

    if (movingPiece->type == PieceType::King) {
        if (movingPiece->color == Color::White) {
            next.castling.whiteKingSide = false;
            next.castling.whiteQueenSide = false;
        } else {
            next.castling.blackKingSide = false;
            next.castling.blackQueenSide = false;
        }
        if (move.category == MoveCategory::KingSideCastle) {
            const std::uint8_t rank = movingPiece->color == Color::White ? 7 : 0;
            board.movePiece(Position{7, rank}, Position{5, rank});
        } else if (move.category == MoveCategory::QueenSideCastle) {
            const std::uint8_t rank = movingPiece->color == Color::White ? 7 : 0;
            board.movePiece(Position{0, rank}, Position{3, rank});
        }
    }

    if (movingPiece->type == PieceType::Rook) {
        disableCastlingForRook(next.castling, movingPiece->color, move.move.from);
    }

    // Handle captures on rook start squares.
    if (move.category == MoveCategory::Capture || move.category == MoveCategory::EnPassant) {
        disableCastlingForCapturedRook(next.castling, move.move.to);
    }

    if (move.category == MoveCategory::DoublePawnPush) {
        const int direction = movingPiece->color == Color::White ? -1 : 1;
        next.enPassantTarget = Position{move.move.from.file, static_cast<std::uint8_t>(move.move.from.rank + direction)};
    }

    if (movingPiece->type == PieceType::Pawn || move.category == MoveCategory::Capture || move.category == MoveCategory::EnPassant) {
        next.halfmoveClock = 0;
    } else {
        ++next.halfmoveClock;
    }

    if (state.sideToMove == Color::Black) {
        ++next.fullmoveNumber;
    }

    next.sideToMove = opposite(state.sideToMove);

    return next;
}

} // namespace chess

