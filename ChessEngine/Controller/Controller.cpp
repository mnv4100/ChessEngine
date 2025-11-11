#include "Controller.h"

#include <optional>
#include <string>
#include <vector>

namespace
{

    std::string squareToNotation(const Vec2 &pos)
    {
        char file = static_cast<char>('a' + pos.x);
        char rank = static_cast<char>('8' - pos.y);
        return {file, rank};
    }

    char pieceToSymbol(uint8_t piece)
    {
        switch (static_cast<PIECE>(piece))
        {
        case PIECE::King:
            return 'K';
        case PIECE::Queen:
            return 'Q';
        case PIECE::Rook:
            return 'R';
        case PIECE::Bishop:
            return 'B';
        case PIECE::Knight:
            return 'N';
        case PIECE::Pion:
            return 'P';
        }
        return '?';
    }

    std::string buildMoveNotation(const Vec2 &from,
                                  const Vec2 &to,
                                  const BoardCell &moving,
                                  const BoardCell &captured,
                                  bool givesCheck)
    {
        const bool isPawn = moving.piece == static_cast<uint8_t>(PIECE::Pion);
        const bool isCapture = captured.fill == 1 && captured.side != moving.side;

        std::string notation;
        if (!isPawn)
        {
            notation.push_back(pieceToSymbol(moving.piece));
        }

        notation += squareToNotation(from);
        notation.push_back(isCapture ? 'x' : '-');
        notation += squareToNotation(to);

        if (givesCheck)
        {
            notation.push_back('+');
        }

        return notation;
    }

} // namespace

// TODO: the board should be rendered only once, and only pieces should be updated

// TODO: Trop focalier sur le roi
// probl�me des tours en early
// Point bleu en dessous du pion a gray
// Quand mouvement impossible enlever point bleu
// Pas de fin de game

void Controller::startGame(Io *io, Core *core, Ai *ai)
{
    // Choose which side the player controls
    SIDE humanSide = SIDE::WHITE_SIDE;
    bool selectionMade = false;
    while (!selectionMade && !io->shouldClose())
    {
        io->beginFrame();
        auto selection = io->renderSideSelectionPrompt();
        if (selection.has_value())
        {
            humanSide = *selection;
            selectionMade = true;
        }
        io->endFrame();
    }

    if (io->shouldClose())
    {
        return;
    }

    io->setPlayerPerspective(humanSide);

    // local variable
    bool hasSelection = false;
    Vec2 selected{};
    auto toMove = SIDE::WHITE_SIDE;
    std::vector<std::string> moveHistory;

    // Which side does the AI play? default to the opposite of the human if ai != nullptr
    SIDE aiSide = (ai != nullptr)
                      ? (humanSide == SIDE::WHITE_SIDE ? SIDE::BLACK_SIDE : SIDE::WHITE_SIDE)
                      : SIDE::WHITE_SIDE;

    // ?? NEW VARIABLE: toggle this for AI vs AI mode
    bool aiVsAi = false; // set to true for AI vs AI, false for Human vs AI

    while (!io->shouldClose())
    {
        io->beginFrame();

        // Check if current player's king is in check
        const Vec2 *checkedKingPos = nullptr;
        Vec2 kingPos;
        if (core->isKingInCheck(toMove))
        {
            kingPos = core->findKing(toMove);
            checkedKingPos = &kingPos;
        }

        const Vec2 *selectedPtr = hasSelection ? &selected : nullptr;
        io->renderChessBoard(*core, checkedKingPos, moveHistory, selectedPtr);

        const bool isAiTurn = ai && (aiVsAi || toMove == aiSide);
        std::string statusMessage;
        if (isAiTurn)
        {
            statusMessage = "AI thinking...";
        }
        else if (toMove == humanSide)
        {
            statusMessage = "Your move.";
        }
        else
        {
            statusMessage = "Waiting for opponent.";
        }
        io->renderGameInfo(toMove, humanSide, isAiTurn, aiVsAi, statusMessage, selectedPtr, hasSelection);

        if (isAiTurn)
        {
            auto optMove = ai->findBestMove(*core, toMove);
            if (optMove)
            {
                const BoardCell movingPiece = core->At(optMove->from);
                const BoardCell capturedPiece = core->At(optMove->to);
                const SIDE opponent = (toMove == SIDE::WHITE_SIDE)
                                          ? SIDE::BLACK_SIDE
                                          : SIDE::WHITE_SIDE;

                if (core->movePiece(optMove->from, optMove->to))
                {
                    bool givesCheck = core->isKingInCheck(opponent);
                    moveHistory.push_back(
                        buildMoveNotation(optMove->from, optMove->to,
                                          movingPiece, capturedPiece, givesCheck));
                    toMove = opponent;
                    hasSelection = false;
                    io->getPossibleMovesToRender().clear();
                }
            }
            io->endFrame();
            continue;
        }

        Vec2 clicked{};
        if (io->consumeBoardClick(clicked))
        {
            if (!hasSelection)
            {
                const BoardCell &cell = core->At(clicked);
                if (cell.fill == 1 && cell.side == static_cast<uint8_t>(toMove))
                {
                    selected = clicked;
                    hasSelection = true;
                    auto possibleMoves = core->getPossibleMoves(selected);
                    auto &toRender = io->getPossibleMovesToRender();
                    toRender = possibleMoves;
                }
            }
            else
            {
                if (clicked.x == selected.x && clicked.y == selected.y)
                {
                    hasSelection = false;
                    io->getPossibleMovesToRender().clear();
                }
                else
                {
                    const BoardCell movingPiece = core->At(selected);
                    const BoardCell capturedPiece = core->At(clicked);
                    const SIDE opponent = (toMove == SIDE::WHITE_SIDE)
                                              ? SIDE::BLACK_SIDE
                                              : SIDE::WHITE_SIDE;
                    if (core->movePiece(selected, clicked))
                    {
                        bool givesCheck = core->isKingInCheck(opponent);
                        moveHistory.push_back(
                            buildMoveNotation(selected, clicked,
                                              movingPiece, capturedPiece, givesCheck));
                        toMove = opponent;
                        hasSelection = false;
                        io->getPossibleMovesToRender().clear();
                    }
                    else
                    {
                        const BoardCell &cell = core->At(clicked);
                        if (cell.fill == 1 && cell.side == static_cast<uint8_t>(toMove))
                        {
                            selected = clicked;
                            hasSelection = true;
                            auto moves = core->getPossibleMoves(selected);
                            auto &toRender = io->getPossibleMovesToRender();
                            toRender = moves;
                        }
                    }
                }
            }
        }

        io->endFrame();
    }
}
