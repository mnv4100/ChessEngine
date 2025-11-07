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
void Io::renderChessBoard(Core& core) const {
	if (chessPieceTexture.id == 0) return;

	const int pieceWidth = chessPieceTexture.width / 6;
	const int pieceHeight = chessPieceTexture.height / 2;

	for (uint8_t y = 0; y < 8; ++y) {
		for (uint8_t x = 0; x < 8; ++x) {
			const BoardCell& cell = core.At(Vec2{ x, y });

			// case pair light gray impart dark
			const Color color = (x + y) % 2 == 0
					? LIGHTGRAY
					: DARKGRAY;

			DrawRectangle(x * cellSize, y * cellSize,cellSize, cellSize, color);

			if (!possibleMovesToRender.empty()) {
				for (auto& move : possibleMovesToRender) {
					if (move.x == x && move.y == y) {
						//DrawRectangle(x * cellSize, y * cellSize, cellSize, cellSize, RED);
						DrawCircle(x * cellSize + cellSize / 2, y * cellSize + cellSize / 2, 10.0f, BLUE);
					}
				}
			}

			// choose which side is going to be at the bottom
			const int row = (cell.side == static_cast<uint8_t>(SIDE::WHITE_SIDE)) ? 1 : 0;

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



