#pragma once

#include "../definition.h"

#include "../Core/Core.h"
#include "../Io/Io.h"

// Implementing a turn black or white can move


class Controller {
public:
    explicit Controller() {};

    ~Controller() {};

    void startGame(Io *io, Core *core);
private:
};
