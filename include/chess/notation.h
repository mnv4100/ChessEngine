#pragma once

#include "chess/types.h"

#include <optional>
#include <string>

namespace chess {

std::optional<Move> parseCoordinateMove(const std::string& text);
std::string toCoordinateNotation(const Move& move);

} // namespace chess

