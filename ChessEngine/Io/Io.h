#pragma once

#include "raylib.h"
#include "Core/Core.h"

class Io {

public:
	explicit Io();
	~Io();

	void renderChessBoard(Core& core) const;

private:
	int windowSizeX = 800;
	int windowSizeY = 800;

	const int cellSize = 100;

	Texture2D chessPieceTexture{};
	Font debugFont{};
};