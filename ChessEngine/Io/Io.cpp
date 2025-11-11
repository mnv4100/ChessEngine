#include "Io.h"

Io::Io()
{
	InitWindow(windowSizeX, windowSizeY, "Super Chess Engine");
	SetTargetFPS(144);

	chessPieceTexture = LoadTexture(ASSETS_PATH "Chess_Pieces_Sprite.svg.png");
	debugFont = LoadFont(ASSETS_PATH "MONARK.otf");
}

Io::~Io()
{
	UnloadTexture(chessPieceTexture);
	UnloadFont(debugFont);
	CloseWindow();
}

// Board
void Io::renderChessBoard(Core& core,
                          const Vec2* checkedKingPos,
                          const std::vector<std::string>& moveHistory) const {
        if (chessPieceTexture.id == 0) return;

        const int pieceWidth = chessPieceTexture.width / 6;
        const int pieceHeight = chessPieceTexture.height / 2;

        for (uint8_t y = 0; y < 8; ++y) {
                for (uint8_t x = 0; x < 8; ++x) {
                        const BoardCell& cell = core.At(Vec2{ x, y });

                        Color color;
                        // Si le roi est en echec et que c'est sa position, colorier en rouge
                        if (checkedKingPos && checkedKingPos->x == x && checkedKingPos->y == y) {
                                color = RED;
                        } else {
                                // case pair light gray impart dark
                                color = (x + y) % 2 == 0 ? LIGHTGRAY : DARKGRAY;
                        }

                        DrawRectangle(x * cellSize, y * cellSize, cellSize, cellSize, color);

                        // choose which side is going to be at the bottom
                        const int row = (cell.side == static_cast<uint8_t>(SIDE::WHITE_SIDE)) ? 1 : 0;

                        if (!possibleMovesToRender.empty()) {
                            for (auto& move : possibleMovesToRender) {
                                if (move.x == x && move.y == y) {
                                    DrawCircle(x * cellSize + cellSize / 2, y * cellSize + cellSize / 2, 10.0f, BLUE);
                                }
                            }
                        }

                        if (cell.fill == 0) continue;

                        const int pieceIndex = cell.piece;

                        const Rectangle srcRect = {
                                static_cast<float>(pieceIndex * pieceWidth),
                                static_cast<float>(row * pieceHeight),
                                static_cast<float>(pieceWidth),
                                static_cast<float>(pieceHeight)
                        };

                        const Rectangle destRect = {
                                static_cast<float>(x) * cellSize,
                                static_cast<float>(y) * cellSize,
                                static_cast<float>(cellSize),
                                static_cast<float>(cellSize)
                        };

                        DrawTexturePro(chessPieceTexture, srcRect, destRect, Vector2{ 0, 0 }, 0.0f, WHITE);
                }
        }

        const int boardPixelSize = cellSize * 8;
        const Rectangle historyBackground{ static_cast<float>(boardPixelSize), 0.0f,
                                           static_cast<float>(windowSizeX - boardPixelSize),
                                           static_cast<float>(windowSizeY) };

        DrawRectangleRec(historyBackground, Fade(LIGHTGRAY, 0.3f));

        const Vector2 titlePos{ static_cast<float>(boardPixelSize) + 20.0f, 20.0f };
        DrawTextEx(debugFont, "Move History", titlePos, 24.0f, 1.0f, BLACK);

        const size_t maxDisplayed = 20;
        const size_t totalMoves = moveHistory.size();
        const size_t startIndex = (totalMoves > maxDisplayed) ? totalMoves - maxDisplayed : 0;

        float textY = titlePos.y + 40.0f;
        for (size_t i = startIndex; i < totalMoves; ++i) {
                const bool whiteMove = (i % 2 == 0);
                const size_t moveNumber = i / 2 + 1;

                std::string entry = std::to_string(moveNumber);
                entry += whiteMove ? ". " : "... ";
                entry += moveHistory[i];

                Vector2 textPos{ titlePos.x, textY };
                DrawTextEx(debugFont, entry.c_str(), textPos, 20.0f, 1.0f, BLACK);
                textY += 24.0f;
        }
}

void Io::getOveredCell(Vec2& cell) const {
	Vector2 m = GetMousePosition();
	int ix = static_cast<int>(m.x) / cellSize;
	int iy = static_cast<int>(m.y) / cellSize;
	if (ix < 0) ix = 0; if (ix > 7) ix = 7;
	if (iy < 0) iy = 0; if (iy > 7) iy = 7;
	cell.x = static_cast<uint8_t>(ix);
	cell.y = static_cast<uint8_t>(iy);
}





