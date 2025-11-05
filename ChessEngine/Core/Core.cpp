#include "Core.h"

#include <iostream>


Core::Core()
{
    fillChessBoard();
}

void Core::debugDisplayChessBoard() const
{
    for (size_t y = 0; y < 8; ++y) {
        std::cout << "\n";
        for (size_t x = 0; x < 8; ++x) {
            std::cout << static_cast<int>(chessBoard[y][x].piece) << " ";
        }
    }
    std::cout << "\n";
}

bool Core::isMoveLegal(const Vec2& from, const Vec2& to) const
{
    if (!isMoveInBounds(from) && !isMoveInBounds(to)) {
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
    int deltaX = abs(to.x - from.x);
    int deltaY = abs(to.y - from.y);

    switch (fromCell.piece) {

    case static_cast<int>(PIECE::Pion): {
        SIDE side = static_cast<SIDE>(fromCell.side);
        int direction = (side == SIDE::WHITE_SIDE) ? 1 : -1;  
        int startRow = (side == SIDE::WHITE_SIDE) ? 1 : 6;    
        int dX = to.x - from.x;
        int dY = to.y - from.y;

        // Avance simple
        if (dX == 0 && dY == direction && toCell.fill == 0) {
            legal = true;
        }
        // Avance double depuis la position de départ
        else if (dX == 0 && dY == 2 * direction && from.y == startRow && toCell.fill == 0) {
            Vec2 intermediate = { from.x, from.y + direction };
            if (At(intermediate).fill == 0) {
                legal = true;
            }
        }
        // Capture en diagonale
        else if (abs(dX) == 1 && dY == direction && toCell.fill == 1) {
            legal = true;
        }
        break;
    }

    case static_cast<int>(PIECE::Rook): {
        // Se déplace horizontalement ou verticalement
        if (deltaX == 0 || deltaY == 0) {
            legal = isPathClear(from, to);
        }
        break;
    }

    case static_cast<int>(PIECE::Knight): {
        // Mouvement en L : 2+1 ou 1+2
        if ((deltaX == 2 && deltaY == 1) || (deltaX == 1 && deltaY == 2)) {
            legal = true; // Le cavalier saute par-dessus
        }
        break;
    }

    case static_cast<int>(PIECE::Bishop): {
        // Se déplace en diagonale
        if (deltaX == deltaY && deltaX > 0) {
            legal = isPathClear(from, to);
        }
        break;
    }   

    case static_cast<int>(PIECE::Queen): {
        // Combine Tour + Fou
        if (deltaX == 0 || deltaY == 0 || deltaX == deltaY) {
            legal = isPathClear(from, to);
        }
        break;
    }

    case static_cast<int>(PIECE::King): {
        // Une seule case dans toutes les directions
        if (deltaX <= 1 && deltaY <= 1 && (deltaX + deltaY > 0)) {
            legal = true;
            // TODO: Vérifier que le roi ne se met pas en échec
            // TODO: Roque
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

    Vec2 current = { from.x + stepX, from.y + stepY };

    while (current.x != to.x || current.y != to.y) {
        if (At(current).fill == 1) {
            return false; // Chemin bloqué
        }
        current.x += stepX;
        current.y += stepY;
    }

    return true;
}

bool Core::movePiece(const Vec2& from, const Vec2& to)
{
    if (!isMoveLegal(from, to)) {
        return false;
    }

    auto& fromCase = At(from);
    auto& toCase = At(to);

    // Copier la pièce vers la destination
    toCase.piece = fromCase.piece;
    toCase.side = fromCase.side;
    toCase.fill = 1;

    // Vider la case source
    fromCase.raw = 0;  

    return true;
}

void Core::fillChessBoard()
{
    for (size_t y = 0; y < 8; ++y) {
        for (size_t x = 0; x < 8; ++x) {

            if (y == 0) { // White back rank
                chessBoard[y][x] = makeCell(white_back_rank[x], SIDE::WHITE_SIDE, true);
            }
            else if (y == 1) { // White pawns
                chessBoard[y][x] = makeCell(PIECE::Pion, SIDE::WHITE_SIDE, true);
            }
            else if (y == 6) { // Black pawns
                chessBoard[y][x] = makeCell(PIECE::Pion, SIDE::BLACK_SIDE, true);
            }
            else if (y == 7) { // Black back rank
                chessBoard[y][x] = makeCell(black_back_rank[x], SIDE::BLACK_SIDE, true);
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

inline BoardCell Core::makeCell(PIECE p, SIDE s, bool occupied)
{
    BoardCell cell{};
    cell.piece = static_cast<uint8_t>(p);
    cell.side = static_cast<uint8_t>(s);
    cell.fill = occupied ? 1 : 0;
    return cell;
}