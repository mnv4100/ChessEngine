# ChessEngine

A modernised C++20 chess engine with both a command-line interface and a real-time, cross-platform SDL2 front-end. The refactor focuses on:

- A clean separation between **data structures** (board, moves) and **game state management**.
- A deterministic rules engine that supports castling, en passant, promotion choices, and check detection.
- A lightweight core library that can back multiple user interfaces.

## Building

```bash
cmake -S . -B build
cmake --build build
```

### Dependencies

The SDL front-end depends on SDL2. On Ubuntu/Debian install it via:

```bash
sudo apt-get install libsdl2-dev
```

On macOS you can use Homebrew:

```bash
brew install sdl2
```

On Windows install the official development package and point `CMAKE_PREFIX_PATH` at the SDL2 CMake config directory.

### Targets

The build produces the following binaries:

- `chess_engine`: a static library containing the board representation and rule enforcement.
- `chess_cli`: a console application that lets you play by entering long algebraic moves such as `e2e4` or `g7g8q`.
- `chess_sdl`: an SDL2 application that renders the board, highlights legal moves, and supports mouse-driven play with on-screen promotion choices.

Run either executable from the build directory, for example:

```bash
./build/chess_cli
./build/chess_sdl
```

## Playing from the console

When the application starts it prints the board in ASCII form. Type moves in coordinate notation (file + rank for the origin and destination). Examples:

- `e2e4` – move the white pawn two squares.
- `g1f3` – develop the knight.
- `e7e8q` – promote a pawn to a queen.

Available commands:

- `help` – show a short summary of commands.
- `reset` – restart from the initial position.
- `fen` – display the current board in ASCII again.
- `quit` / `exit` – close the program.

The engine reports check, checkmate, and stalemate directly in the console.

## Project layout

```
include/chess      Public headers for the chess library and UIs
src/               Engine implementation plus CLI and SDL2 front-ends
CMakeLists.txt     Single target build configuration
```

The core engine remains dependency-free so it can be embedded into other projects (GUI front-ends, network services, or AI search) without extra baggage.

## Next steps

- Add automated tests (perft suites, move validation cases).
- Expose a FEN parser/exporter for interoperability.
- Implement a basic AI searcher that uses the new rule engine.

Contributions and issue reports are welcome!

