#include "chess/sdl_ui.h"

#include "chess/board.h"
#include "chess/types.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <optional>
#include <stdexcept>
#include <utility>

namespace chess {

	namespace {

		struct Glyph {
			std::array<std::uint8_t, 7> rows{};
		};

		constexpr int kGlyphWidth = 5;
		constexpr int kGlyphHeight = 7;

		constexpr std::array<std::pair<PieceType, Glyph>, 6> kPieceGlyphs{ {
			{PieceType::Pawn,   Glyph{{0b01110, 0b10001, 0b10001, 0b11111, 0b10000, 0b10000, 0b10000}}},
			{PieceType::Knight, Glyph{{0b10001, 0b10011, 0b10101, 0b11001, 0b10101, 0b10011, 0b10001}}},
			{PieceType::Bishop, Glyph{{0b01110, 0b10001, 0b10010, 0b01100, 0b10010, 0b10001, 0b01110}}},
			{PieceType::Rook,   Glyph{{0b11111, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001}}},
			{PieceType::Queen,  Glyph{{0b01110, 0b10001, 0b10001, 0b10101, 0b10001, 0b10001, 0b01110}}},
			{PieceType::King,   Glyph{{0b10001, 0b10001, 0b10101, 0b01110, 0b10101, 0b10001, 0b10001}}},
		} };

		[[nodiscard]] const Glyph& glyphForPiece(PieceType type) {
			for (const auto& [pieceType, glyph] : kPieceGlyphs) {
				if (pieceType == type) {
					return glyph;
				}
			}
			return kPieceGlyphs.front().second;
		}

		// SDL3 rendering APIs use SDL_FRect
		void drawGlyph(SDL_Renderer* renderer, const Glyph& glyph, SDL_FRect bounds, SDL_Color color) {
			const float pixelWidth = std::max(1.0f, bounds.w / static_cast<float>(kGlyphWidth));
			const float pixelHeight = std::max(1.0f, bounds.h / static_cast<float>(kGlyphHeight));
			const float glyphWidth = pixelWidth * static_cast<float>(kGlyphWidth);
			const float glyphHeight = pixelHeight * static_cast<float>(kGlyphHeight);
			const float offsetX = bounds.x + (bounds.w - glyphWidth) / 2.0f;
			const float offsetY = bounds.y + (bounds.h - glyphHeight) / 2.0f;

			SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
			for (int row = 0; row < kGlyphHeight; ++row) {
				const std::uint8_t mask = glyph.rows[row];
				for (int col = 0; col < kGlyphWidth; ++col) {
					if ((mask & (0b10000 >> col)) == 0) {
						continue;
					}
					SDL_FRect pixel{
						offsetX + static_cast<float>(col) * pixelWidth,
						offsetY + static_cast<float>(row) * pixelHeight,
						pixelWidth,
						pixelHeight,
					};
					SDL_RenderFillRect(renderer, &pixel);
				}
			}
		}

		[[nodiscard]] SDL_Color pieceFill(Color color) {
			return color == Color::White ? SDL_Color{ 235, 235, 235, 255 } : SDL_Color{ 20, 20, 20, 255 };
		}

		[[nodiscard]] SDL_Color pieceStroke(Color color) {
			return color == Color::White ? SDL_Color{ 20, 20, 20, 255 } : SDL_Color{ 235, 235, 235, 255 };
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

		// SDL3: SDL_Init returns bool (true on success, false on failure)
		if (!SDL_Init(SDL_INIT_VIDEO)) {
			throw std::runtime_error(std::string{ "Failed to initialise SDL: " } + SDL_GetError());
		}

		// SDL3 SDL_CreateWindow(title, w, h, flags)
		window_ = SDL_CreateWindow("Chess Engine", windowWidth_, windowHeight_, SDL_WINDOW_MINIMIZED);
		if (!window_) {
			SDL_Quit();
			throw std::runtime_error(std::string{ "Failed to create window: " } + SDL_GetError());
		}

		// SDL3 SDL_CreateRenderer(window, name) – NULL lets SDL choose a driver
		renderer_ = SDL_CreateRenderer(window_, nullptr);
		if (!renderer_) {
			SDL_DestroyWindow(window_);
			SDL_Quit();
			throw std::runtime_error(std::string{ "Failed to create renderer: " } + SDL_GetError());
		}

		SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
		refreshLegalMoves();
	}

	SdlUI::~SdlUI() {
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

	void SdlUI::run() {
		running_ = true;
		while (running_) {
			SDL_Event event;
			// SDL3: SDL_PollEvent returns bool
			while (SDL_PollEvent(&event)) {
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

	void SdlUI::render() {
		SDL_SetRenderDrawColor(renderer_, 15, 15, 20, 255);
		SDL_RenderClear(renderer_);

		const int boardSize = std::min(windowWidth_, windowHeight_);
		const int boardX = (windowWidth_ - boardSize) / 2;
		const int boardY = (windowHeight_ - boardSize) / 2;
		const int squareSize = boardSize / 8;

		renderBoard(boardX, boardY, squareSize);
		renderPromotionMenu(boardX, boardY, squareSize);

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
					targetRect.x += static_cast<float>(squareSize / 4);
					targetRect.y += static_cast<float>(squareSize / 4);
					targetRect.w = static_cast<float>(squareSize / 2);
					targetRect.h = static_cast<float>(squareSize / 2);
					SDL_RenderFillRect(renderer_, &targetRect);
				}

				const auto& piece = board.at(pos);
				if (!piece) {
					continue;
				}

				const auto fill = pieceFill(piece->color);
				SDL_SetRenderDrawColor(renderer_, fill.r, fill.g, fill.b, fill.a);
				SDL_RenderFillRect(renderer_, &rect);

				const auto stroke = pieceStroke(piece->color);
				drawGlyph(renderer_, glyphForPiece(piece->type), rect, stroke);
			}
		}
	}

	void SdlUI::renderPromotionMenu(int boardX, int boardY, int squareSize) {
		if (!promotionMenu_) {
			return;
		}

		const auto buttons = layoutPromotionButtons(promotionMenu_->to, promotionMenu_->options, boardX, boardY, squareSize);
		for (const auto& button : buttons) {
			SDL_SetRenderDrawColor(renderer_, 30, 30, 30, 230);
			SDL_RenderFillRect(renderer_, &button.rect);
			SDL_SetRenderDrawColor(renderer_, 220, 220, 220, 255);
			SDL_RenderRect(renderer_, &button.rect);
			drawGlyph(renderer_, glyphForPiece(button.piece), button.rect, SDL_Color{ 255, 255, 255, 255 });
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
