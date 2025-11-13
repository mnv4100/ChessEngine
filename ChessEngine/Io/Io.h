#pragma once

#include "Core/Core.h"

#include <optional>
#include <string>
#include <vector>

#include <imgui.h>
#include <glad/gl.h>

struct GLFWwindow;

class Io {

public:
        explicit Io();
        ~Io();

        void beginFrame();
        void endFrame();
        [[nodiscard]] bool shouldClose() const;

        [[nodiscard]] std::optional<SIDE> renderSideSelectionPrompt();

        void refactorRenderChessBoard(Core *core);

        void renderChessBoard(Core& core,
                              const Vec2* checkedKingPos = nullptr,
                              const std::vector<std::string>& moveHistory = {},
                              const Vec2* selectedCell = nullptr);

        void renderGameInfo(SIDE toMove,
                            SIDE humanSide,
                            bool aiTurn,
                            bool aiVsAi,
                            const std::string& statusMessage,
                            const Vec2* selectedCell = nullptr,
                            bool hasSelection = false);

        [[nodiscard]] bool getOveredCell(Vec2& cell) const;
        [[nodiscard]] bool consumeBoardClick(Vec2& cell) const;

        void setPlayerPerspective(SIDE side) { 
            sidePerspective = side;
        }


        // get a reference to the possibleMovesToRender;
        [[nodiscard]] std::vector<Vec2> &getPossibleMovesToRender() { return possibleMovesToRender; }

private:
        int windowSizeX = 800;
        int windowSizeY = 800;

        ImVec2 WindowSize {800, 800};

        const int cellSize = 100;

        std::vector<Vec2> possibleMovesToRender;

        GLFWwindow* window = nullptr;

        ImVec2 boardOrigin{0.0f, 0.0f};
        float boardCellSize = 0.0f;
        bool boardLayoutValid = false;

        ImVec4 clearColor{0.1f, 0.1f, 0.1f, 1.0f};

        bool whitePerspective = true;
		SIDE sidePerspective = SIDE::WHITE_SIDE;

        bool loadPieceSprites();
        void destroyPieceSprites();

        [[nodiscard]] bool hasPieceSprites() const { return pieceTexture != 0; }

        GLuint pieceTexture = 0;
        ImVec2 pieceTextureSize{0.0f, 0.0f};
        ImVec2 pieceTileSize{0.0f, 0.0f};

        void buildDockspace();

        [[nodiscard]] Vec2 toBoardCoordinates(const Vec2& displayCell) const;
        [[nodiscard]] Vec2 toDisplayCoordinates(const Vec2& boardCell) const;
};