#include "chess/sdl_ui.h"

#include "chess/board.h"
#include "chess/types.h"

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

#ifndef CHESS_ASSETS_DIR
#define CHESS_ASSETS_DIR "."
#endif

namespace chess {

	namespace {

		constexpr int kSpriteColumns = 6;
		constexpr int kSpriteRows = 2;
		constexpr float kPiecePaddingRatio = 0.12f;

		[[nodiscard]] int spriteColumnForPiece(PieceType type) {
			switch (type) {
			case PieceType::King:   return 0;
			case PieceType::Queen:  return 1;
			case PieceType::Bishop: return 2;
			case PieceType::Knight: return 3;
			case PieceType::Rook:   return 4;
			case PieceType::Pawn:   return 5;
			default:                return 0;
			}
		}

		[[nodiscard]] SDL_Color pieceTint(Color color) {
			return color == Color::White ? SDL_Color{ 235, 235, 235, 255 } : SDL_Color{ 35, 35, 35, 255 };
		}

		[[nodiscard]] SDL_Color squareColor(int file, int rank) {
			const bool light = (file + rank) % 2 == 0;
			return light ? SDL_Color{ 240, 217, 181, 255 } : SDL_Color{ 181, 136, 99, 255 };
		}

		[[nodiscard]] SDL_Color highlightColor() {
			return SDL_Color{ 120, 170, 60, 150 };
		}

		[[nodiscard]] SDL_Color captureHighlightColor() {
			return SDL_Color{ 186, 66, 66, 160 };
		}

		// SDL3 render rects: use SDL_FRect
		[[nodiscard]] SDL_FRect squareRect(int boardX, int boardY, int squareSize, const Position& pos) {
			return SDL_FRect{
				static_cast<float>(boardX + static_cast<int>(pos.file) * squareSize),
				static_cast<float>(boardY + static_cast<int>(pos.rank) * squareSize),
				static_cast<float>(squareSize),
				static_cast<float>(squareSize)
			};
		}

		[[nodiscard]] std::vector<CategorisedMove> filterMovesFrom(
			const std::vector<CategorisedMove>& moves,
			const Position& from) {
			std::vector<CategorisedMove> result;
			for (const auto& move : moves) {
				if (move.move.from == from) {
					result.push_back(move);
				}
			}
			return result;
		}

		[[nodiscard]] std::vector<CategorisedMove> filterMovesTo(
			const std::vector<CategorisedMove>& moves,
			const Position& to) {
			std::vector<CategorisedMove> result;
			for (const auto& move : moves) {
				if (move.move.to == to) {
					result.push_back(move);
				}
			}
			return result;
		}

		struct PromotionButton {
			PieceType piece;
			SDL_FRect rect;
		};

		[[nodiscard]] std::vector<PromotionButton> layoutPromotionButtons(
			const Position& to,
			const std::vector<PieceType>& options,
			int boardX,
			int boardY,
			int squareSize)
		{
			const SDL_FRect base = squareRect(boardX, boardY, squareSize, to);
			const int buttonSize = std::max(32, squareSize / 2);
			const int padding = 6;

			float x = base.x + base.w + static_cast<float>(padding);
			if (x + static_cast<float>(buttonSize) > static_cast<float>(boardX + 8 * squareSize)) {
				x = base.x - static_cast<float>(padding + buttonSize);
			}
			float y = base.y;

			std::vector<PromotionButton> buttons;
			buttons.reserve(options.size());
			for (PieceType piece : options) {
				SDL_FRect rect{
					x,
					y,
					static_cast<float>(buttonSize),
					static_cast<float>(buttonSize)
				};
				buttons.push_back(PromotionButton{ piece, rect });
				y += static_cast<float>(buttonSize + padding);
			}
			return buttons;
		}

	} // namespace

	SdlUI::SdlUI(Game game)
		: game_(std::move(game)) {

		if (!SDL_Init(SDL_INIT_VIDEO)) {
			throw std::runtime_error(std::string{ "Failed to initialise SDL: " } + SDL_GetError());
		}

		window_ = SDL_CreateWindow("Chess Engine", windowWidth_, windowHeight_, NULL);
		if (!window_) {
			SDL_Quit();
			throw std::runtime_error(std::string{ "Failed to create window: " } + SDL_GetError());
		}

		renderer_ = SDL_CreateRenderer(window_, NULL);
		if (!renderer_) {
			SDL_DestroyWindow(window_);
			SDL_Quit();
			throw std::runtime_error(std::string{ "Failed to create renderer: " } + SDL_GetError());
		}

		SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);

		setupImGui();

		try {
			loadAssets();
		}
		catch (...) {
			destroyAssets();
			SDL_DestroyRenderer(renderer_);
			SDL_DestroyWindow(window_);
			SDL_Quit();
			throw;
		}

		refreshLegalMoves();
	}

	SdlUI::~SdlUI() {
		destroyAssets();
		
		shutdownImGui();
		
		if (renderer_) {
			SDL_DestroyRenderer(renderer_);
			renderer_ = nullptr;
		}
		if (window_) {
			SDL_DestroyWindow(window_);
			window_ = nullptr;
		}
		SDL_Quit();

	}

	void SdlUI::setupImGui()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		// TODO: Add checks for failures
		ImGui_ImplSDL3_InitForSDLRenderer(window_, renderer_);
		ImGui_ImplSDLRenderer3_Init(renderer_);
	}

	void SdlUI::shutdownImGui()
	{
		ImGui_ImplSDLRenderer3_Shutdown();
		ImGui_ImplSDL3_Shutdown();
		ImGui::DestroyContext();
	}

	void SdlUI::loadAssets() {
		auto* surface = IMG_Load(CHESS_ASSETS_DIR "/chesspieces.svg");
		
		if (surface == NULL) {
			throw std::runtime_error(std::string{ "Failed to load chess pieces surface from " } + CHESS_ASSETS_DIR "/chesspieces.svg" + ": " + SDL_GetError());
		}

		spriteCellWidth_ = surface->w / kSpriteColumns;
		spriteCellHeight_ = surface->h / kSpriteRows;
		if (spriteCellWidth_ <= 0 || spriteCellHeight_ <= 0) {
			SDL_DestroySurface(surface);
			spriteCellWidth_ = 0;
			spriteCellHeight_ = 0;
			throw std::runtime_error("Chess pieces texture has invalid dimensions");
		}

		SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);
		SDL_DestroySurface(surface);
		if (!texture) {
			spriteCellWidth_ = 0;
			spriteCellHeight_ = 0;
			throw std::runtime_error(std::string{ "Failed to create chess pieces texture: " } + SDL_GetError());
		}

		pieceTexture_ = texture;
	}

	void SdlUI::destroyAssets() {
		if (pieceTexture_) {
			SDL_DestroyTexture(pieceTexture_);
			pieceTexture_ = nullptr;
		}
		spriteCellWidth_ = 0;
		spriteCellHeight_ = 0;
	}

	SDL_FRect SdlUI::spriteSourceRect(PieceType type, Color color) const {
		const int column = spriteColumnForPiece(type);
		const int row = color == Color::White ? 0 : 1;
		return SDL_FRect{
			static_cast<float>(column * spriteCellWidth_),
			static_cast<float>(row * spriteCellHeight_),
			static_cast<float>(spriteCellWidth_),
			static_cast<float>(spriteCellHeight_)
		};
	}

	void SdlUI::drawPieceSprite(PieceType type, Color color, SDL_FRect bounds) {
		if (!pieceTexture_ || spriteCellWidth_ <= 0 || spriteCellHeight_ <= 0) {
			return;
		}

		const SDL_FRect src = spriteSourceRect(type, color);
		const SDL_Color tint = pieceTint(color);
		SDL_SetTextureColorMod(pieceTexture_, tint.r, tint.g, tint.b);

		const float spriteAspect = static_cast<float>(spriteCellWidth_) / static_cast<float>(spriteCellHeight_);
		const float paddingW = bounds.w * kPiecePaddingRatio;
		const float paddingH = bounds.h * kPiecePaddingRatio;
		const float maxWidth = std::max(bounds.w - paddingW, 1.0f);
		const float maxHeight = std::max(bounds.h - paddingH, 1.0f);

		float destWidth = maxWidth;
		float destHeight = destWidth / spriteAspect;
		if (destHeight > maxHeight) {
			destHeight = maxHeight;
			destWidth = destHeight * spriteAspect;
		}

		SDL_FRect dest{
			bounds.x + (bounds.w - destWidth) / 2.0f,
			bounds.y + (bounds.h - destHeight) / 2.0f,
			destWidth,
			destHeight
		};

		SDL_RenderTexture(renderer_, pieceTexture_, &src, &dest);
	}

	void SdlUI::run() {
		running_ = true;
		while (running_) {
			SDL_Event event;
			while (SDL_PollEvent(&event)) {
				ImGui_ImplSDL3_ProcessEvent(&event);
				handleEvent(event);
			}
			render();
			SDL_Delay(16);
		}
	}

	void SdlUI::refreshLegalMoves() {
		legalMoves_ = game_.legalMoves();
		if (selected_) {
			movesFromSelection_ = filterMovesFrom(legalMoves_, *selected_);
		}
		else {
			movesFromSelection_.clear();
		}
	}

	void SdlUI::handleEvent(const SDL_Event& event) {
		switch (event.type) {
		case SDL_EVENT_QUIT:
			running_ = false;
			break;

		case SDL_EVENT_MOUSE_BUTTON_DOWN:
			handleMouseButton(event.button.x, event.button.y);
			break;

		case SDL_EVENT_WINDOW_RESIZED:
			windowWidth_ = event.window.data1;
			windowHeight_ = event.window.data2;
			break;

		default:
			break;
		}
	}

	void SdlUI::handleMouseButton(int x, int y) {
		const int boardSize = std::min(windowWidth_, windowHeight_);
		const int boardX = (windowWidth_ - boardSize) / 2;
		const int boardY = (windowHeight_ - boardSize) / 2;
		const int squareSize = boardSize / 8;

		if (promotionMenu_) {
			const auto buttons = layoutPromotionButtons(promotionMenu_->to, promotionMenu_->options, boardX, boardY, squareSize);
			for (const auto& button : buttons) {
				if (x >= button.rect.x && x < button.rect.x + button.rect.w &&
					y >= button.rect.y && y < button.rect.y + button.rect.h) {
					for (const auto& move : movesFromSelection_) {
						if (move.move.to == promotionMenu_->to && move.promotion && *move.promotion == button.piece) {
							tryExecuteMove(move);
							promotionMenu_.reset();
							return;
						}
					}
				}
			}
			// Click outside menu cancels promotion selection but keeps piece selected.
			promotionMenu_.reset();
			return;
		}

		const auto boardPos = mapPixelToBoard(boardX, boardY, squareSize, x, y);
		if (boardPos) {
			handleSquareClick(*boardPos);
		}
		else {
			selected_.reset();
			movesFromSelection_.clear();
		}
	}

	void SdlUI::handleSquareClick(const Position& boardPos) {
		const auto& board = game_.state().board;
		const auto piece = board.at(boardPos);
		const auto sideToMove = game_.state().sideToMove;

		if (selected_) {
			const auto matchingMoves = filterMovesTo(movesFromSelection_, boardPos);
			if (!matchingMoves.empty()) {
				if (matchingMoves.size() == 1) {
					tryExecuteMove(matchingMoves.front());
				}
				else {
					PromotionMenu menu{ *selected_, boardPos, {} };
					for (const auto& move : matchingMoves) {
						if (move.promotion) {
							menu.options.push_back(*move.promotion);
						}
					}
					promotionMenu_ = menu;
				}
				return;
			}

			if (piece && piece->color == sideToMove) {
				selected_ = boardPos;
				movesFromSelection_ = filterMovesFrom(legalMoves_, *selected_);
				promotionMenu_.reset();
				return;
			}

			selected_.reset();
			movesFromSelection_.clear();
			promotionMenu_.reset();
			return;
		}

		if (piece && piece->color == sideToMove) {
			selected_ = boardPos;
			movesFromSelection_ = filterMovesFrom(legalMoves_, *selected_);
			promotionMenu_.reset();
		}
	}

	void SdlUI::tryExecuteMove(const CategorisedMove& move) {
		if (game_.tryMove(move.move, move.promotion)) {
			selected_.reset();
			movesFromSelection_.clear();
			promotionMenu_.reset();
			refreshLegalMoves();
		}
	}

	void SdlUI::imGuiRender()
	{
		// Start ImGui frame
		ImGui_ImplSDLRenderer3_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();

		// Debug Window
		ImGui::Begin("Engine Controls");
		
		if (ImGui::Button("Reset Game")) {
			game_.reset();
			selected_.reset();
			movesFromSelection_.clear();
			promotionMenu_.reset();
			refreshLegalMoves();
		}

		ImGui::Text("Side to move: %s", game_.state().sideToMove == Color::White ? "White" : "Black");

		ImGui::End();
		ImGui::Render();
		ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer_);
	}

	void SdlUI::render() {
		SDL_SetRenderDrawColor(renderer_, 15, 15, 20, 255);
		SDL_RenderClear(renderer_);

		const int boardSize = std::min(windowWidth_, windowHeight_);
		const int boardX = (windowWidth_ - boardSize) / 2;
		const int boardY = (windowHeight_ - boardSize) / 2;
		const int squareSize = boardSize / 8;

		renderBoard(boardX, boardY, squareSize);
		renderPromotionMenu(boardX, boardY, squareSize);
		imGuiRender();
		SDL_RenderPresent(renderer_);
	}

	void SdlUI::renderBoard(int boardX, int boardY, int squareSize) {
		const auto& board = game_.state().board;

		for (int rank = 0; rank < 8; ++rank) {
			for (int file = 0; file < 8; ++file) {
				Position pos{ static_cast<std::uint8_t>(file), static_cast<std::uint8_t>(rank) };
				const SDL_FRect rect = squareRect(boardX, boardY, squareSize, pos);
				const auto baseColor = squareColor(file, rank);
				SDL_SetRenderDrawColor(renderer_, baseColor.r, baseColor.g, baseColor.b, baseColor.a);
				SDL_RenderFillRect(renderer_, &rect);

				const bool isSelected = selected_ && *selected_ == pos;
				if (isSelected) {
					SDL_SetRenderDrawColor(renderer_, 80, 120, 220, 160);
					SDL_RenderFillRect(renderer_, &rect);
				}

				const auto iter = std::find_if(
					movesFromSelection_.begin(),
					movesFromSelection_.end(),
					[&](const CategorisedMove& move) { return move.move.to == pos; });
				if (iter != movesFromSelection_.end()) {
					const bool isCapture = iter->category == MoveCategory::Capture ||
						iter->category == MoveCategory::EnPassant ||
						iter->category == MoveCategory::Promotion;
					const auto highlight = isCapture ? captureHighlightColor() : highlightColor();
					SDL_SetRenderDrawColor(renderer_, highlight.r, highlight.g, highlight.b, highlight.a);
					SDL_FRect targetRect = rect;
					
					// make it a bit smaller than full square
					//targetRect.x += static_cast<float>(squareSize / 4);
					//targetRect.y += static_cast<float>(squareSize / 4);
					//targetRect.w = static_cast<float>(squareSize / 2);
					//targetRect.h = static_cast<float>(squareSize / 2);
					
					SDL_RenderFillRect(renderer_, &targetRect);
				}

				const auto& piece = board.at(pos);
				if (!piece) {
					continue;
				}

				drawPieceSprite(piece->type, piece->color, rect);
			}
		}
	}

	void SdlUI::renderPromotionMenu(int boardX, int boardY, int squareSize) {
		if (!promotionMenu_) {
			return;
		}

		const auto buttons = layoutPromotionButtons(promotionMenu_->to, promotionMenu_->options, boardX, boardY, squareSize);
		const Color promoteColor = game_.state().sideToMove;
		for (const auto& button : buttons) {
			SDL_SetRenderDrawColor(renderer_, 30, 30, 30, 230);
			SDL_RenderFillRect(renderer_, &button.rect);
			SDL_SetRenderDrawColor(renderer_, 220, 220, 220, 255);
			SDL_RenderRect(renderer_, &button.rect);
			drawPieceSprite(button.piece, promoteColor, button.rect);
		}
	}

	std::optional<Position> SdlUI::mapPixelToBoard(int boardX, int boardY, int squareSize, int x, int y) const {
		if (squareSize <= 0) {
			return std::nullopt;
		}
		const int boardSize = squareSize * 8;
		if (x < boardX || y < boardY || x >= boardX + boardSize || y >= boardY + boardSize) {
			return std::nullopt;
		}
		const int file = (x - boardX) / squareSize;
		const int rank = (y - boardY) / squareSize;
		if (file < 0 || file >= 8 || rank < 0 || rank >= 8) {
			return std::nullopt;
		}
		return Position{ static_cast<std::uint8_t>(file), static_cast<std::uint8_t>(rank) };
	}

} // namespace chess
