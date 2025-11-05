
#include "Io.h"

Io::Io()
{
	InitWindow(windowSizeX, windowSizeY, "Super Chess Engine");
	SetTargetFPS(60);

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

			const Color color = ((x + y) % 2 == 0) ? LIGHTGRAY : DARKGRAY;

			DrawRectangle(x * cellSize, y * cellSize,cellSize, cellSize, color);
			const int row = (cell.side == static_cast<uint8_t>(SIDE::WHITE_SIDE)) ? 0 : 1;

			if (cell.fill == 0) continue;

			const int pieceIndex = (int)cell.piece;

			const Rectangle srcRect = {
				(float)(pieceIndex * pieceWidth),
				(float)(row * pieceHeight),
				(float)pieceWidth,
				(float)pieceHeight
			};

			Rectangle destRect = {
				(float)x * cellSize,
				(float)y * cellSize,
				(float)cellSize,
				(float)cellSize
			};

			DrawTexturePro(chessPieceTexture, srcRect, destRect, Vector2{ 0, 0 }, 0.0f, WHITE);
		}
	}
}



