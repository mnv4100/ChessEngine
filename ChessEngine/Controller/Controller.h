#pragma once

#include "../definition.h"

#include "../Core/Core.h"
#include "../Io/Io.h"

class Controller {
public:
    explicit Controller() {};

    ~Controller() {};

    void startGame(Io *io, Core *core) {
        io->renderChessBoard(*core);
    }
private:
};
