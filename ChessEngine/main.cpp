#include "definition.h"

#include "Core/Core.h"
#include "Io/Io.h"
#include "Controller/Controller.h"

int main() {
    Core core;
    Io io;
    Controller controller;

    controller.startGame(&io, &core);

    return 0;
}