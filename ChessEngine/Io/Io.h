#pragma once

#include "raylib.h"
#include "Core/Core.h"

class Io {

public:
	explicit Io();
	~Io();

	void renderChessBoard(Core& core, const Vec2* checkedKingPos = nullptr) const;
	void getOveredCell(Vec2& cell) const;

	// get a reference to the possibleMovesToRender;
	[[nodiscard]] std::vector<Vec2> &getPossibleMovesToRender() { return possibleMovesToRender; }

private:
	int windowSizeX = 800;
	int windowSizeY = 800;

	const int cellSize = 100;

	std::vector<Vec2> possibleMovesToRender;

	Texture2D chessPieceTexture{};
	Font debugFont{};
};