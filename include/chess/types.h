#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <ostream>
#include <string>

namespace chess {

	/**
	 * A chessboard is represented using 0-based coordinates where files (columns)
	 * are counted from the left (file `0` represents column `a`) and ranks
	 * (rows) are counted from the top (rank `0` represents the eighth rank).
	 */
	struct Position {
		std::uint8_t file{ 0 };
		std::uint8_t rank{ 0 };

		[[nodiscard]] bool isValid() const noexcept {
			return file < 8 && rank < 8;
		}
	};

	inline bool operator==(const Position& lhs, const Position& rhs) noexcept {
		return lhs.file == rhs.file && lhs.rank == rhs.rank;
	}

	inline bool operator!=(const Position& lhs, const Position& rhs) noexcept {
		return !(lhs == rhs);
	}

	enum class Color : std::uint8_t {
		White,
		Black
	};

	inline Color opposite(Color color) noexcept {
		return color == Color::White ? Color::Black : Color::White;
	}

	enum class PieceType : std::uint8_t {
		Pawn,
		Knight,
		Bishop,
		Rook,
		Queen,
		King
	};

	struct Piece {
		PieceType type{};
		Color color{};
	};

	struct Move {
		Position from{};
		Position to{};
		std::optional<PieceType> promotion{};
	};

	inline bool operator==(const Move& lhs, const Move& rhs) {
		return lhs.from == rhs.from && lhs.to == rhs.to && lhs.promotion == rhs.promotion;
	}

	enum class MoveCategory : std::uint8_t {
		Quiet,
		Capture,
		DoublePawnPush,
		KingSideCastle,
		QueenSideCastle,
		EnPassant,
		Promotion
	};

	struct CategorisedMove {
		Move move{};
		MoveCategory category{ MoveCategory::Quiet };
		std::optional<PieceType> promotion{};
	};

	constexpr std::array<char, 6> kPieceToChar{ {'P', 'N', 'B', 'R', 'Q', 'K'} };

	inline std::string to_string(Color color) {
		return color == Color::White ? "white" : "black";
	}

	inline std::ostream& operator<<(std::ostream& os, Color color) {
		return os << to_string(color);
	}

	inline std::string to_string(const Position& pos) {
		if (!pos.isValid()) {
			return "??";
		}
		std::string result;
		result.push_back(static_cast<char>('a' + pos.file));
		result.push_back(static_cast<char>('8' - pos.rank));
		return result;
	}

} // namespace chess

