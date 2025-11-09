#include "Controller.h"

#include "raylib.h"


// TODO: the board should be rendered only once, and only pieces should be updated

void Controller::startGame(Io *io, Core *core) {
    bool hasSelection = false;
    
    Vec2 selected{};
    
    SIDE toMove = SIDE::WHITE_SIDE;

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
        io->renderChessBoard(*core, checkedKingPos);
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
                    bool moved = core->movePiece(selected, clicked);
                    if (moved) {
                        
                        toMove = (toMove == SIDE::WHITE_SIDE) ? SIDE::BLACK_SIDE : SIDE::WHITE_SIDE;
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
