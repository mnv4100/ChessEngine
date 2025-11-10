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

void Controller::startGame(Io* io, Core* core, Ai* ai) {
    bool hasSelection = false;
    Vec2 selected{};
    SIDE toMove = SIDE::WHITE_SIDE;
    std::vector<std::string> moveHistory;

    // Which side does the AI play? default to BLACK if ai != nullptr
    SIDE aiSide = (ai != nullptr) ? SIDE::BLACK_SIDE : SIDE::WHITE_SIDE;

    // ?? NEW VARIABLE: toggle this for AI vs AI mode
    bool aiVsAi = true; // set to true for AI vs AI, false for Human vs AI

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Check if current player's king is in check
        Vec2* checkedKingPos = nullptr;
        Vec2 kingPos;
        if (core->isKingInCheck(toMove)) {
            kingPos = core->findKing(toMove);
            checkedKingPos = &kingPos;
        }

        // Render board and pieces
        io->renderChessBoard(*core, checkedKingPos, moveHistory);
        DrawFPS(10, 10);

        // === AI Turn ===
        bool isAiTurn = ai && (aiVsAi || toMove == aiSide);
        if (isAiTurn) {
            const char* thinking = "AI thinking...";
            int textW = MeasureText(thinking, 20);
            DrawText(thinking, (GetScreenWidth() - textW) / 2, 10, 20, DARKGRAY);
            EndDrawing();

            // optional: small wait to visualize turns
            WaitTime(0.05f);

            auto optMove = ai->findBestMove(*core, toMove);
            if (optMove) {
                const BoardCell movingPiece = core->At(optMove->from);
                const BoardCell capturedPiece = core->At(optMove->to);
                const SIDE opponent = (toMove == SIDE::WHITE_SIDE)
                    ? SIDE::BLACK_SIDE
                    : SIDE::WHITE_SIDE;

                if (core->movePiece(optMove->from, optMove->to)) {
                    bool givesCheck = core->isKingInCheck(opponent);
                    moveHistory.push_back(
                        buildMoveNotation(optMove->from, optMove->to,
                            movingPiece, capturedPiece, givesCheck));
                    toMove = opponent;
                }
            }
            continue;
        }

        // === Human Input (only if aiVsAi == false) ===
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vec2 clicked{};
            io->getOveredCell(clicked);

            if (!hasSelection) {
                const BoardCell& cell = core->At(clicked);
                if (cell.fill == 1 && cell.side == static_cast<uint8_t>(toMove)) {
                    selected = clicked;
                    hasSelection = true;
                    auto possibleMoves = core->getPossibleMoves(selected);
                    auto& toRender = io->getPossibleMovesToRender();
                    toRender = possibleMoves;
                }
            }
            else {
                if (clicked.x == selected.x && clicked.y == selected.y) {
                    hasSelection = false;
                    io->getPossibleMovesToRender().clear();
                }
                else {
                    const BoardCell movingPiece = core->At(selected);
                    const BoardCell capturedPiece = core->At(clicked);
                    const SIDE opponent = (toMove == SIDE::WHITE_SIDE)
                        ? SIDE::BLACK_SIDE
                        : SIDE::WHITE_SIDE;
                    if (core->movePiece(selected, clicked)) {
                        bool givesCheck = core->isKingInCheck(opponent);
                        moveHistory.push_back(
                            buildMoveNotation(selected, clicked,
                                movingPiece, capturedPiece, givesCheck));
                        toMove = opponent;
                        hasSelection = false;
                        io->getPossibleMovesToRender().clear();
                    }
                    else {
                        const BoardCell& cell = core->At(clicked);
                        if (cell.fill == 1 && cell.side == static_cast<uint8_t>(toMove)) {
                            selected = clicked;
                            hasSelection = true;
                            auto moves = core->getPossibleMoves(selected);
                            auto& toRender = io->getPossibleMovesToRender();
                            toRender = moves;
                        }
                    }
                }
            }
        }

        EndDrawing();
    }
}
