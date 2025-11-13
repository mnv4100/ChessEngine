#pragma once

#include "../definition.h"

#include "../Core/Core.h"
#include "../Io/Io.h"
#include "../Core/Ai.h"

namespace Controller {
	void startGame(Io* io, Core* core, Ai* ai);
	void refactor_startGame(Io *io, Core *core, Ai *ai);
}