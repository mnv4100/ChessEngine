#pragma once

#include "raylib.h"
#include "Core/Core.h"

#include <string>
#include <vector>

class Io {

public:
        explicit Io();
        ~Io();

        void renderChessBoard(Core& core,
                              const Vec2* checkedKingPos = nullptr,
                              const std::vector<std::string>& moveHistory = {}) const;
        void getOveredCell(Vec2& cell) const;

        void setPlayerPerspective(SIDE side) { whitePerspective = (side == SIDE::WHITE_SIDE); }

        // get a reference to the possibleMovesToRender;
        [[nodiscard]] std::vector<Vec2> &getPossibleMovesToRender() { return possibleMovesToRender; }

private:
        int windowSizeX = 1100;
        int windowSizeY = 800;

        const int cellSize = 100;

        std::vector<Vec2> possibleMovesToRender;

        Texture2D chessPieceTexture{};
        Font debugFont{};

        bool whitePerspective = true;

        [[nodiscard]] Vec2 toBoardCoordinates(const Vec2& displayCell) const;
        [[nodiscard]] Vec2 toDisplayCoordinates(const Vec2& boardCell) const;
};