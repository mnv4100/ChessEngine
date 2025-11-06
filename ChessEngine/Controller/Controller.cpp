
#include "Controller.h"
#include "raylib.h"


void Controller::startGame(Io *io, Core *core) {
    // Game state
    bool hasSelection = false;
    Vec2 selected{};
    SIDE toMove = SIDE::WHITE_SIDE;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Render board and pieces
        io->renderChessBoard(*core);

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
                        // Switch turn
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
