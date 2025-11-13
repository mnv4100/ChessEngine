#pragma once

#include "chess/game.h"

#include <SDL3/SDL.h>

#include <optional>
#include <vector>

namespace chess {

class SdlUI {
public:
    explicit SdlUI(Game game);
    ~SdlUI();

    SdlUI(const SdlUI&) = delete;
    SdlUI& operator=(const SdlUI&) = delete;
    SdlUI(SdlUI&&) = delete;
    SdlUI& operator=(SdlUI&&) = delete;

    void run();

private:
    struct PromotionMenu {
        Position from{};
        Position to{};
        std::vector<PieceType> options;
    };

    Game game_;
    SDL_Window* window_{nullptr};
    SDL_Renderer* renderer_{nullptr};
    bool running_{false};
    int windowWidth_{800};
    int windowHeight_{800};

    std::vector<CategorisedMove> legalMoves_;
    std::optional<Position> selected_;
    std::vector<CategorisedMove> movesFromSelection_;
    std::optional<PromotionMenu> promotionMenu_;

    void handleEvent(const SDL_Event& event);
    void handleMouseButton(int x, int y);
    void handleSquareClick(const Position& boardPos);
    void tryExecuteMove(const CategorisedMove& move);
    void refreshLegalMoves();

    void render();
    void renderBoard(int boardX, int boardY, int squareSize);
    void renderPromotionMenu(int boardX, int boardY, int squareSize);

    [[nodiscard]] std::optional<Position> mapPixelToBoard(int boardX, int boardY, int squareSize, int x, int y) const;
};

} // namespace chess

