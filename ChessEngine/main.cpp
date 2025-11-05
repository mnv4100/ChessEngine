#include "definition.h"

#include "Core/Core.h"
#include "Io/Io.h"
#include "Controller/Controller.h"

#include <iostream>
#include "raylib.h"

int main() {
	
	Core core;

	Vec2 pion = { 1, 6 };
	Vec2 target = { 1, 5 };

	Vec2 fou = { 2, 7 };
	Vec2 targetFou = { 0, 5 }; // x y
	
	std::cout << core.movePiece(pion, target) << std::endl;
	std::cout << core.movePiece(fou, targetFou) << std::endl;

	std::cout.clear();

	core.debugDisplayChessBoard();

	Io io;
	Controller controller;

	while (!WindowShouldClose()) {

		BeginDrawing();


		controller.startGame(&io, &core);

		EndDrawing();
	}




	
	
	return 0;
}