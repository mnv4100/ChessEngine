#include "Core/Core.h"
#include "Io/Io.h"
#include "Controller/Controller.h"
#include "Core/Ai.h"

int main() {
    Core core;
    Io io;
    Controller controller;
    Ai ai(&core, SIDE::BLACK_SIDE);

    controller.startGame(&io, &core, &ai);

    return 0;
}


