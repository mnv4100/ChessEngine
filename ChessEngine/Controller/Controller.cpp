#include "Controller.h"

#include "raylib.h"

#include <string>
#include <vector>


namespace {

std::string squareToNotation(const Vec2& pos) {
    char file = static_cast<char>('a' + pos.x);
    char rank = static_cast<char>('8' - pos.y);
    return { file, rank };
}

char pieceToSymbol(uint8_t piece) {
    switch (static_cast<PIECE>(piece)) {
        case PIECE::King: return 'K';
        case PIECE::Queen: return 'Q';
        case PIECE::Rook: return 'R';
        case PIECE::Bishop: return 'B';
        case PIECE::Knight: return 'N';
        case PIECE::Pion: return 'P';
    }
    return '?';
}

std::string buildMoveNotation(const Vec2& from,
                              const Vec2& to,
                              const BoardCell& moving,
                              const BoardCell& captured,
                              bool givesCheck) {
    const bool isPawn = moving.piece == static_cast<uint8_t>(PIECE::Pion);
    const bool isCapture = captured.fill == 1 && captured.side != moving.side;

    std::string notation;
    if (!isPawn) {
        notation.push_back(pieceToSymbol(moving.piece));
    }

    notation += squareToNotation(from);
    notation.push_back(isCapture ? 'x' : '-');
    notation += squareToNotation(to);

    if (givesCheck) {
        notation.push_back('+');
    }

    return notation;
}

} // namespace


// TODO: the board should be rendered only once, and only pieces should be updated

void Controller::startGame(Io *io, Core *core) {
    bool hasSelection = false;

    Vec2 selected{};

    SIDE toMove = SIDE::WHITE_SIDE;
    std::vector<std::string> moveHistory;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Check if current player's king is in check
        Vec2 *checkedKingPos = nullptr;
        Vec2 kingPos;
        if (core->isKingInCheck(toMove)) {
            kingPos = core->findKing(toMove);
            checkedKingPos = &kingPos;
        }

        // Render board and pieces
        io->renderChessBoard(*core, checkedKingPos, moveHistory);
        // Debug: show FPS
        DrawFPS(10, 10);
        
        // Handle input
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vec2 clicked{};
            io->getOveredCell(clicked);

            if (!hasSelection) {
                const BoardCell &cell = core->At(clicked);
                if (cell.fill == 1 && cell.side == static_cast<uint8_t>(toMove)) {
                    selected = clicked;
                    hasSelection = true;
                    // Compute and push possible moves to Io for rendering
                    auto possibleMoves = core->getPossibleMoves(selected);
                    auto &toRender = io->getPossibleMovesToRender();
                    toRender = possibleMoves;
                }
            } else {
                // If clicking the same square, deselect
                if (clicked.x == selected.x && clicked.y == selected.y) {
                    hasSelection = false;
                    // Clear highlights
                    auto &toRender = io->getPossibleMovesToRender();
                    toRender.clear();
                } else {
                    // Try moving
                    const BoardCell movingPiece = core->At(selected);
                    const BoardCell capturedPiece = core->At(clicked);
                    const SIDE opponent = (toMove == SIDE::WHITE_SIDE) ? SIDE::BLACK_SIDE : SIDE::WHITE_SIDE;
                    bool moved = core->movePiece(selected, clicked);
                    if (moved) {

                        bool givesCheck = core->isKingInCheck(opponent);
                        moveHistory.push_back(buildMoveNotation(selected, clicked, movingPiece, capturedPiece, givesCheck));

                        toMove = opponent;
                        hasSelection = false;
                        // Clear highlights after move
                        auto &toRender = io->getPossibleMovesToRender();
                        toRender.clear();
                    } else {
                        // If move failed but clicked on own piece, change selection
                        const BoardCell &cell = core->At(clicked);
                        if (cell.fill == 1 && cell.side == static_cast<uint8_t>(toMove)) {
                            selected = clicked;
                            hasSelection = true;
                            // Recompute possible moves for new selection
                            auto moves = core->getPossibleMoves(selected);
                            auto &toRender = io->getPossibleMovesToRender();
                            toRender = moves;
                        }
                    }
                }
            }
        }

        EndDrawing();
    }
}
