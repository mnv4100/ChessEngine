#include "Core/Core.h"
#include "Io/Io.h"
#include "Controller/Controller.h"
#include "Core/Ai.h"

int main() {
    Core core;
    Io io;
    Ai ai(&core);
   
    Controller::startGame(&io, &core, &ai);

    return 0;
}


