#include "chess/notation.h"

#include <algorithm>
#include <cctype>

namespace chess {
namespace {

bool parseFile(char c, std::uint8_t& out) {
    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    if (c < 'a' || c > 'h') {
        return false;
    }
    out = static_cast<std::uint8_t>(c - 'a');
    return true;
}

bool parseRank(char c, std::uint8_t& out) {
    if (c < '1' || c > '8') {
        return false;
    }
    out = static_cast<std::uint8_t>(8 - (c - '0'));
    return true;
}

std::optional<PieceType> parsePromotion(char c) {
    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    switch (c) {
    case 'q':
        return PieceType::Queen;
    case 'r':
        return PieceType::Rook;
    case 'b':
        return PieceType::Bishop;
    case 'n':
        return PieceType::Knight;
    default:
        return std::nullopt;
    }
}

char promotionToChar(PieceType type) {
    switch (type) {
    case PieceType::Queen:
        return 'q';
    case PieceType::Rook:
        return 'r';
    case PieceType::Bishop:
        return 'b';
    case PieceType::Knight:
        return 'n';
    default:
        return 'q';
    }
}

} // namespace

std::optional<Move> parseCoordinateMove(const std::string& text) {
    if (text.size() < 4) {
        return std::nullopt;
    }

    std::uint8_t fromFile{};
    std::uint8_t fromRank{};
    std::uint8_t toFile{};
    std::uint8_t toRank{};

    if (!parseFile(text[0], fromFile) || !parseRank(text[1], fromRank) ||
        !parseFile(text[2], toFile) || !parseRank(text[3], toRank)) {
        return std::nullopt;
    }

    Move move{};
    move.from = Position{fromFile, fromRank};
    move.to = Position{toFile, toRank};

    if (text.size() == 5) {
        move.promotion = parsePromotion(text[4]);
        if (!move.promotion) {
            return std::nullopt;
        }
    }

    return move;
}

std::string toCoordinateNotation(const Move& move) {
    std::string result;
    result.reserve(move.promotion ? 5 : 4);
    result.push_back(static_cast<char>('a' + move.from.file));
    result.push_back(static_cast<char>('8' - move.from.rank));
    result.push_back(static_cast<char>('a' + move.to.file));
    result.push_back(static_cast<char>('8' - move.to.rank));

    if (move.promotion) {
        result.push_back(promotionToChar(*move.promotion));
    }

    return result;
}

} // namespace chess

