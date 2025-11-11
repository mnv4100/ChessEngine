#include "Core.h"

#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <optional>
#include <utility>


Core::Core()
{
    fillChessBoard();
    setupCache();
}


// if the cell is filled push it to cache 
void Core::setupCache() {
    filledCell.clear();
    filledCell.reserve(32);
    for (size_t y = 0; y < 8; ++y) {
        for (size_t x = 0; x < 8; ++x) {
            size_t index = y * 8 + x;
            if (chessBoard[index].fill == 1) {
                filledCell.push_back(Vec2{ static_cast<uint8_t>(x), static_cast<uint8_t>(y) });
            }
        }
    }
}

void Core::removeFromCache(const Vec2& pos)
{
    auto it = std::remove(filledCell.begin(), filledCell.end(), pos);
    if (it != filledCell.end()) {
        filledCell.erase(it, filledCell.end());
    }
}

void Core::updateCache(const Vec2& from,
    const Vec2& to,
    bool capturedDestination,
    std::optional<Vec2> enPassantCaptured,
    std::optional<std::pair<Vec2, Vec2>> rookMove)
{
    removeFromCache(from);
    if (capturedDestination) {
        removeFromCache(to);
    }
    if (enPassantCaptured.has_value()) {
        removeFromCache(*enPassantCaptured);
    }
    if (rookMove.has_value()) {
        removeFromCache(rookMove->first);
        filledCell.push_back(rookMove->second);
    }
    filledCell.push_back(to);
}

void Core::debugDisplayChessBoard() const
{
    for (size_t y = 0; y < 8; ++y) {
        std::cout << "\n";
        for (size_t x = 0; x < 8; ++x) {
            std::cout << static_cast<int>(chessBoard[y * 8 + x].piece ) << " ";
        }
    }
    std::cout << "\n";
}


bool Core::isMoveLegal(const Vec2& from, const Vec2& to) const
{
    if (!isMoveInBounds(from) || !isMoveInBounds(to)) {
        std::cout << "Move out of bounds\n";
        return false;
    }

    const BoardCell& fromCell = At(from);
    const BoardCell& toCell = At(to);

    if (fromCell.fill == 0) {
        std::cout << "No piece at the from position";
        return false;
    }

    if (toCell.fill == 1 && fromCell.side == toCell.side) {
        std::cout << "Can't Capture ur own piece\n";
        return false;
    }

    bool legal = false;
    int dX = static_cast<int>(to.x) - static_cast<int>(from.x);
    int dY = static_cast<int>(to.y) - static_cast<int>(from.y);
    int deltaX = std::abs(dX);
    int deltaY = std::abs(dY);

    const SIDE movingSide = static_cast<SIDE>(fromCell.side);
    const SIDE opponentSide = (movingSide == SIDE::WHITE_SIDE) ? SIDE::BLACK_SIDE : SIDE::WHITE_SIDE;

    switch (fromCell.piece) {

    case static_cast<int>(PIECE::Pion): {
        int direction = (movingSide == SIDE::WHITE_SIDE) ? -1 : 1;
        int startRow = (movingSide == SIDE::WHITE_SIDE) ? 6 : 1;

        if (dX == 0 && dY == direction && toCell.fill == 0) {
            legal = true;
        }
        else if (dX == 0 && dY == 2 * direction && from.y == startRow && toCell.fill == 0) {
            Vec2 intermediate = { static_cast<uint8_t>(from.x), static_cast<uint8_t>(from.y + direction) };
            if (At(intermediate).fill == 0) {
                legal = true;
            }
        }
        else if (deltaX == 1 && dY == direction && toCell.fill == 1 && toCell.side != fromCell.side) {
            legal = true;
        }
        else if (deltaX == 1 && dY == direction && toCell.fill == 0 && enPassantActive) {
            if (to.x == enPassantTarget.x && to.y == enPassantTarget.y) {
                const BoardCell& captured = At(enPassantCapturedPawn);
                if (captured.fill == 1 &&
                    captured.piece == static_cast<uint8_t>(PIECE::Pion) &&
                    captured.side != fromCell.side) {
                    legal = true;
                }
            }
        }
        break;
    }

    case static_cast<int>(PIECE::Rook): {
        if (deltaX == 0 || deltaY == 0) {
            legal = isPathClear(from, to);
        }
        break;
    }

    case static_cast<int>(PIECE::Knight): {
        if ((deltaX == 2 && deltaY == 1) || (deltaX == 1 && deltaY == 2)) {
            legal = true;
        }
        break;
    }

    case static_cast<int>(PIECE::Bishop): {
        if (deltaX == deltaY && deltaX > 0) {
            legal = isPathClear(from, to);
        }
        break;
    }

    case static_cast<int>(PIECE::Queen): {
        if (deltaX == 0 || deltaY == 0 || deltaX == deltaY) {
            legal = isPathClear(from, to);
        }
        break;
    }

    case static_cast<int>(PIECE::King): {
        if (deltaX <= 1 && deltaY <= 1 && (deltaX + deltaY > 0)) {
            legal = true;
        }
        else if (deltaY == 0 && deltaX == 2) {
            bool kingSide = (dX > 0);
            bool kingHasMoved = (movingSide == SIDE::WHITE_SIDE) ? whiteKingMoved : blackKingMoved;
            if (!kingHasMoved && !hasRookMoved(movingSide, kingSide)) {
                Vec2 rookPos{ static_cast<uint8_t>(kingSide ? 7 : 0), from.y };
                const BoardCell& rookCell = At(rookPos);
                if (rookCell.fill == 1 &&
                    rookCell.piece == static_cast<uint8_t>(PIECE::Rook) &&
                    rookCell.side == fromCell.side) {
                    if (isPathClear(from, rookPos) && At(to).fill == 0) {
                        Vec2 stepSquare{ static_cast<uint8_t>(from.x + (kingSide ? 1 : -1)), from.y };
                        if (!isSquareAttacked(from, opponentSide) &&
                            !isSquareAttacked(stepSquare, opponentSide) &&
                            !isSquareAttacked(to, opponentSide)) {
                            legal = true;
                        }
                    }
                }
            }
        }
        break;
    }
    }

    return legal;
}

// Fonction helper pour vérifier qu'il n'y a pas de pièce entre from et to
bool Core::isPathClear(const Vec2& from, const Vec2& to) const
{
    int stepX = (to.x > from.x) ? 1 : (to.x < from.x) ? -1 : 0;
    int stepY = (to.y > from.y) ? 1 : (to.y < from.y) ? -1 : 0;

    Vec2 current = { static_cast<uint8_t>(from.x + stepX), static_cast<uint8_t>(from.y + stepY) };

    while (current.x != to.x || current.y != to.y) {
        if (At(current).fill == 1) {
            return false; // Chemin bloqué
        }
        current.x += stepX;
        current.y += stepY;
    }

    return true;
}

// Function to find king position
Vec2 Core::findKing(SIDE side) const {
    for (uint8_t y = 0; y < 8; ++y) {
        for (uint8_t x = 0; x < 8; ++x) {
            const BoardCell& cell = At({x, y});
            if (cell.fill == 1 && 
                cell.side == static_cast<uint8_t>(side) && 
                cell.piece == static_cast<uint8_t>(PIECE::King)) {
                return {x, y};
            }
        }
    }
    // This should never happen in a valid chess game
    return {0, 0};
}

bool Core::isKingInCheck(SIDE kingSide) const {
    Vec2 kingPos = findKing(kingSide);
    SIDE opponentSide = (kingSide == SIDE::WHITE_SIDE) ? SIDE::BLACK_SIDE : SIDE::WHITE_SIDE;
    return isSquareAttacked(kingPos, opponentSide);
}

bool Core::movePiece(const Vec2& from, const Vec2& to) {
    if (!isMoveLegal(from, to)) {
        return false;
    }

    // Store the original state to restore if the move puts own king in check
    BoardCell originalFrom = At(from);
    BoardCell originalTo = At(to);

    bool originalWhiteKingMoved = whiteKingMoved;
    bool originalBlackKingMoved = blackKingMoved;
    bool originalWhiteRookMoved[2] = { whiteRookMoved[0], whiteRookMoved[1] };
    bool originalBlackRookMoved[2] = { blackRookMoved[0], blackRookMoved[1] };
    bool originalEnPassantActive = enPassantActive;
    Vec2 originalEnPassantTarget = enPassantTarget;
    Vec2 originalEnPassantCapturedPawn = enPassantCapturedPawn;

    bool isCastlingMove = (originalFrom.piece == static_cast<uint8_t>(PIECE::King) &&
                           std::abs(static_cast<int>(to.x) - static_cast<int>(from.x)) == 2 &&
                           from.y == to.y);

    bool isEnPassantCapture = (originalFrom.piece == static_cast<uint8_t>(PIECE::Pion) &&
                               enPassantActive &&
                               to.x == enPassantTarget.x &&
                               to.y == enPassantTarget.y &&
                               originalTo.fill == 0);

    Vec2 rookFromPos{};
    Vec2 rookToPos{};
    BoardCell rookFromOriginal{};
    BoardCell rookToOriginal{};
    bool adjustRook = false;

    if (isCastlingMove) {
        bool kingSide = (to.x > from.x);
        rookFromPos = { static_cast<uint8_t>(kingSide ? 7 : 0), from.y };
        rookToPos = { static_cast<uint8_t>(kingSide ? 5 : 3), from.y };
        rookFromOriginal = At(rookFromPos);
        rookToOriginal = At(rookToPos);
        adjustRook = true;
    }

    BoardCell enPassantCapturedOriginal{};
    if (isEnPassantCapture) {
        enPassantCapturedOriginal = At(enPassantCapturedPawn);
    }

    // Make the move
    auto& fromCase = At(from);
    auto& toCase = At(to);

    const bool capturedDestination = (originalTo.fill == 1);
    std::optional<Vec2> enPassantCaptured = std::nullopt;
    if (isEnPassantCapture) {
        enPassantCaptured = enPassantCapturedPawn;
    }
    std::optional<std::pair<Vec2, Vec2>> rookMoveInfo = std::nullopt;
    if (adjustRook) {
        rookMoveInfo = std::make_pair(rookFromPos, rookToPos);
    }

    toCase.piece = fromCase.piece;
    toCase.side = fromCase.side;
    toCase.fill = 1;
    fromCase.raw = 0;

    if (isEnPassantCapture) {
        At(enPassantCapturedPawn).raw = 0;
    }

    if (adjustRook) {
        auto& rookFromCase = At(rookFromPos);
        auto& rookToCase = At(rookToPos);
        rookToCase = rookFromCase;
        rookFromCase.raw = 0;
    }

    // Check if the move puts/leaves own king in check
    SIDE movingSide = static_cast<SIDE>(originalFrom.side);

    if (originalFrom.piece == static_cast<uint8_t>(PIECE::King)) {
        if (movingSide == SIDE::WHITE_SIDE) {
            whiteKingMoved = true;
        } else {
            blackKingMoved = true;
        }
        if (isCastlingMove) {
            markRookMoved(movingSide, to.x > from.x);
        }
    }

    if (originalFrom.piece == static_cast<uint8_t>(PIECE::Rook)) {
        if (from.y == (movingSide == SIDE::WHITE_SIDE ? 7 : 0)) {
            if (from.x == 0) {
                markRookMoved(movingSide, false);
            } else if (from.x == 7) {
                markRookMoved(movingSide, true);
            }
        }
    }

    if (originalTo.fill == 1 &&
        originalTo.piece == static_cast<uint8_t>(PIECE::Rook) &&
        originalTo.side != originalFrom.side) {
        handleRookCapture(to, static_cast<SIDE>(originalTo.side));
    }

    enPassantActive = false;
    if (originalFrom.piece == static_cast<uint8_t>(PIECE::Pion)) {
        int direction = (movingSide == SIDE::WHITE_SIDE) ? -1 : 1;
        if (static_cast<int>(to.y) - static_cast<int>(from.y) == 2 * direction) {
            enPassantActive = true;
            enPassantTarget = { static_cast<uint8_t>(from.x), static_cast<uint8_t>(from.y + direction) };
            enPassantCapturedPawn = to;
        }
    }

    if (isKingInCheck(movingSide)) {
        // Restore the original position
        At(from) = originalFrom;
        At(to) = originalTo;
        if (isEnPassantCapture) {
            At(enPassantCapturedPawn) = enPassantCapturedOriginal;
        }
        if (adjustRook) {
            At(rookFromPos) = rookFromOriginal;
            At(rookToPos) = rookToOriginal;
        }
        // std::cout << "Move would put/leave own king in check\n";
        whiteKingMoved = originalWhiteKingMoved;
        blackKingMoved = originalBlackKingMoved;
        whiteRookMoved[0] = originalWhiteRookMoved[0];
        whiteRookMoved[1] = originalWhiteRookMoved[1];
        blackRookMoved[0] = originalBlackRookMoved[0];
        blackRookMoved[1] = originalBlackRookMoved[1];
        enPassantActive = originalEnPassantActive;
        enPassantTarget = originalEnPassantTarget;
        enPassantCapturedPawn = originalEnPassantCapturedPawn;
        return false;
    }

    if (originalFrom.piece == static_cast<uint8_t>(PIECE::Pion)) {
        if ((movingSide == SIDE::WHITE_SIDE && to.y == 0) ||
            (movingSide == SIDE::BLACK_SIDE && to.y == 7)) {
            BoardCell& promoted = At(to);
            promoted.piece = static_cast<uint8_t>(PIECE::Queen);
        }
    }

    updateCache(from, to, capturedDestination, enPassantCaptured, rookMoveInfo);

    return true;
}

std::vector<Vec2> Core::getPossibleMoves(const Vec2 &from) const {
    std::vector<Vec2> moves;

    // Validate source square
    if (!isMoveInBounds(from)) return moves;
    const BoardCell &fromCell = At(from);
    if (fromCell.fill == 0) return moves;

    const SIDE movingSide = static_cast<SIDE>(fromCell.side);

    auto tryAddMove = [&](int targetX, int targetY) {
        if (targetX < 0 || targetX >= 8 || targetY < 0 || targetY >= 8) {
            return;
        }

        Vec2 to{ static_cast<uint8_t>(targetX), static_cast<uint8_t>(targetY) };
        const BoardCell &targetCell = At(to);
        if (targetCell.fill == 1 && targetCell.side == fromCell.side) {
            return;
        }

        if (isMoveLegal(from, to)) {
            moves.push_back(to);
        }
    };

    auto addSlidingMoves = [&](int stepX, int stepY) {
        int currentX = static_cast<int>(from.x) + stepX;
        int currentY = static_cast<int>(from.y) + stepY;

        while (currentX >= 0 && currentX < 8 && currentY >= 0 && currentY < 8) {
            Vec2 to{ static_cast<uint8_t>(currentX), static_cast<uint8_t>(currentY) };
            const BoardCell &targetCell = At(to);

            if (targetCell.fill == 1 && targetCell.side == fromCell.side) {
                break;
            }

            if (isMoveLegal(from, to)) {
                moves.push_back(to);
            }

            if (targetCell.fill == 1) {
                break;
            }

            currentX += stepX;
            currentY += stepY;
        }
    };

    switch (static_cast<PIECE>(fromCell.piece)) {
    case PIECE::Pion: {
        int direction = (movingSide == SIDE::WHITE_SIDE) ? -1 : 1;
        int startRow = (movingSide == SIDE::WHITE_SIDE) ? 6 : 1;

        // Single step forward
        int forwardY = static_cast<int>(from.y) + direction;
        if (forwardY >= 0 && forwardY < 8) {
            Vec2 forward{ from.x, static_cast<uint8_t>(forwardY) };
            if (At(forward).fill == 0) {
                tryAddMove(forward.x, forward.y);

                // Double step from starting rank
                if (static_cast<int>(from.y) == startRow) {
                    int doubleForwardY = forwardY + direction;
                    if (doubleForwardY >= 0 && doubleForwardY < 8) {
                        Vec2 doubleForward{ from.x, static_cast<uint8_t>(doubleForwardY) };
                        if (At(doubleForward).fill == 0) {
                            tryAddMove(doubleForward.x, doubleForward.y);
                        }
                    }
                }
            }
        }

        // Captures (including en passant handled by isMoveLegal)
        tryAddMove(static_cast<int>(from.x) - 1, static_cast<int>(from.y) + direction);
        tryAddMove(static_cast<int>(from.x) + 1, static_cast<int>(from.y) + direction);
        break;
    }
    case PIECE::Knight: {
        static constexpr int offsets[8][2] = {
            { 1, 2 },{ 2, 1 },{ 2, -1 },{ 1, -2 },
            { -1, -2 },{ -2, -1 },{ -2, 1 },{ -1, 2 }
        };
        for (const auto &offset : offsets) {
            tryAddMove(static_cast<int>(from.x) + offset[0], static_cast<int>(from.y) + offset[1]);
        }
        break;
    }
    case PIECE::Bishop: {
        addSlidingMoves(1, 1);
        addSlidingMoves(1, -1);
        addSlidingMoves(-1, 1);
        addSlidingMoves(-1, -1);
        break;
    }
    case PIECE::Rook: {
        addSlidingMoves(1, 0);
        addSlidingMoves(-1, 0);
        addSlidingMoves(0, 1);
        addSlidingMoves(0, -1);
        break;
    }
    case PIECE::Queen: {
        addSlidingMoves(1, 0);
        addSlidingMoves(-1, 0);
        addSlidingMoves(0, 1);
        addSlidingMoves(0, -1);
        addSlidingMoves(1, 1);
        addSlidingMoves(1, -1);
        addSlidingMoves(-1, 1);
        addSlidingMoves(-1, -1);
        break;
    }
    case PIECE::King: {
        for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = -1; dy <= 1; ++dy) {
                if (dx == 0 && dy == 0) continue;
                tryAddMove(static_cast<int>(from.x) + dx, static_cast<int>(from.y) + dy);
            }
        }

        // Castling moves are validated inside isMoveLegal
        tryAddMove(static_cast<int>(from.x) + 2, static_cast<int>(from.y));
        tryAddMove(static_cast<int>(from.x) - 2, static_cast<int>(from.y));
        break;
    }
    }

    return moves;
}



void Core::fillChessBoard()
{
    for (size_t y = 0; y < 8; ++y) {
        for (size_t x = 0; x < 8; ++x) {
            size_t index = y * 8 + x;

            if (y == 7) { // White back rank at bottom
                chessBoard[index] = makeCell(white_back_rank[x], SIDE::WHITE_SIDE, true);
            }
            else if (y == 6) { // White pawns
                chessBoard[index] = makeCell(PIECE::Pion, SIDE::WHITE_SIDE, true);
            }
            else if (y == 1) { // Black pawns
                chessBoard[index] = makeCell(PIECE::Pion, SIDE::BLACK_SIDE, true);
            }
            else if (y == 0) { // Black back rank at top
                chessBoard[index] = makeCell(black_back_rank[x], SIDE::BLACK_SIDE, true);
            }
            else { // Empty squares
            }
        }
    }
}

inline bool Core::isMoveInBounds(const Vec2& cell) const
{
    return cell.x < 8 && cell.y < 8;
}

bool Core::isSquareAttacked(const Vec2& square, SIDE bySide) const
{
    for (uint8_t y = 0; y < 8; ++y) {
        for (uint8_t x = 0; x < 8; ++x) {
            Vec2 from{ x, y };
            const BoardCell& cell = At(from);
            if (cell.fill == 0 || cell.side != static_cast<uint8_t>(bySide)) {
                continue;
            }

            int dX = static_cast<int>(square.x) - static_cast<int>(from.x);
            int dY = static_cast<int>(square.y) - static_cast<int>(from.y);
            int deltaX = std::abs(dX);
            int deltaY = std::abs(dY);

            switch (static_cast<PIECE>(cell.piece)) {
            case PIECE::Pion: {
                int direction = (bySide == SIDE::WHITE_SIDE) ? -1 : 1;
                if (dY == direction && deltaX == 1) {
                    return true;
                }
                break;
            }
            case PIECE::Knight: {
                if ((deltaX == 1 && deltaY == 2) || (deltaX == 2 && deltaY == 1)) {
                    return true;
                }
                break;
            }
            case PIECE::Bishop: {
                if (deltaX == deltaY && deltaX > 0) {
                    if (isPathClear(from, square)) {
                        return true;
                    }
                }
                break;
            }
            case PIECE::Rook: {
                if (deltaX == 0 || deltaY == 0) {
                    if (isPathClear(from, square)) {
                        return true;
                    }
                }
                break;
            }
            case PIECE::Queen: {
                if (deltaX == deltaY || deltaX == 0 || deltaY == 0) {
                    if (isPathClear(from, square)) {
                        return true;
                    }
                }
                break;
            }
            case PIECE::King: {
                if (deltaX <= 1 && deltaY <= 1 && (deltaX + deltaY > 0)) {
                    return true;
                }
                break;
            }
            }
        }
    }

    return false;
}

bool Core::hasRookMoved(SIDE side, bool kingSide) const
{
    const bool* rookArray = (side == SIDE::WHITE_SIDE) ? whiteRookMoved : blackRookMoved;
    return rookArray[kingSide ? 1 : 0];
}

void Core::markRookMoved(SIDE side, bool kingSide)
{
    bool* rookArray = (side == SIDE::WHITE_SIDE) ? whiteRookMoved : blackRookMoved;
    rookArray[kingSide ? 1 : 0] = true;
}

void Core::handleRookCapture(const Vec2& pos, SIDE capturedSide)
{
    uint8_t homeRank = (capturedSide == SIDE::WHITE_SIDE) ? 7 : 0;
    if (pos.y != homeRank) {
        return;
    }

    if (pos.x == 0) {
        markRookMoved(capturedSide, false);
    } else if (pos.x == 7) {
        markRookMoved(capturedSide, true);
    }
}

constexpr inline BoardCell Core::makeCell(PIECE p, SIDE s, bool occupied) noexcept {

    BoardCell cell {};

    // 3 bits are still not used
    // TODO: Removing the flag to use only 4 bits so can align a 4 bits value
    constexpr uint8_t PIECE_MASK = 0b0000'0111; 
    constexpr uint8_t SIDE_MASK = 0b0000'1000;  
    constexpr uint8_t FILL_MASK = 0b0001'0000;  
    // Combine all fields into one byte
    
    cell.raw =
        (static_cast<uint8_t>(p) & PIECE_MASK) |  
        ((static_cast<uint8_t>(s) << 3) & SIDE_MASK) | 
        (occupied ? FILL_MASK : 0);                   

    return cell;
}

