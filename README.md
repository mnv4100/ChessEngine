# ChessEngine

A lightweight **C++23 chess engine** featuring **real-time rendering** via **Dear ImGui** (backed by **GLFW/OpenGL**), built with a **cross-platform CMake + Ninja** setup.
The project runs on **Windows**, **macOS**, and **Linux**, with minimal dependencies and modular architecture for clarity and scalability.

**Transparency note:** Parts of this documentation were drafted with AI assistance, but the entire codebase and design decisions are fully my own.

---

## Features

- **8x8 board stored in a 1D array**, enabling efficient CPU cache utilization.
- **Mouse-based piece selection and move highlighting**.
- **Immediate-mode UI rendering** powered by **Dear ImGui** on top of GLFW/OpenGL.
- **Modular architecture** organized into three main components:
  - `Core`: Rules, data structures, and move generation.
  - `Io`: Input and rendering through Dear ImGui + GLFW.
  - `Controller`: Turn management and game flow logic.
- Fully deterministic updates and consistent frame timing.

### Limitations / TODOs
- Improve **move legality validation** (check, checkmate, stalemate detection).
- Implement **castling**, **en passant**, and **promotion choice**.
- Optimize **move generation** and **data layout** for performance.
- Integrate **AI search (minimax, alpha-beta pruning)**.
- Add **`perft`** validation and **unit tests** for correctness.

---

## Project Structure

```
ChessEngine/
├── Core/          # Core chess logic (rules, move generation, board representation)
├── Io/            # Rendering, input handling, window management (Dear ImGui + GLFW)
├── Controller/    # Game state management, player turns, logic loop
├── assets/        # Piece textures and UI resources
├── main.cpp       # Application entry point
├── CMakeLists.txt # Root configuration (downloads Dear ImGui/GLFW/glad via FetchContent)
└── CMakePresets.json # Presets for fast build setup on Windows
```

The build system automatically fetches **Dear ImGui**, **GLFW**, and **glad** on first configuration.

---

## Requirements

- **CMake** ≥ 3.10  
- **C++23-capable compiler**:
  - Windows: MSVC (Visual Studio or Build Tools)
  - macOS: Clang (via Xcode Command Line Tools)
  - Linux: g++ or Clang
- **Ninja** (recommended for speed)
- **Git** (required to clone Dear ImGui/GLFW/glad via FetchContent)

### Quick Verification

```bash
cmake --version
ninja --version
git --version
```

If these commands return valid versions, you’re good to go.

---

## Building the Project

### Platform-specific notes

- **Windows**
  - Install the *Desktop development with C++* workload (MSVC, Windows SDK).
  - Use the bundled CMake presets (`cmake --preset x64-debug`).
- **macOS**
  - Requires the Xcode Command Line Tools (`xcode-select --install`).
  - The OpenGL backend targets macOS 10.13+ using GLSL 1.50.
- **Linux**
  - Install the X11 and OpenGL development headers before configuring:

    ```bash
    sudo apt-get install libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev \
         libgl1-mesa-dev mesa-common-dev
    ```

  - Run `cmake -S . -B build && cmake --build build`.

From the repository root you can always do:

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

This will produce the executable inside the `build/` directory.

---

## Troubleshooting

| Issue | Cause / Solution |
|-------|------------------|
| `cl.exe` not found | Open a “Developer Command Prompt” or install MSVC Build Tools. |
| `ninja` not found | Install with `choco install ninja` (Windows) or `brew install ninja` (macOS). |
| ImGui/GLFW fetch fails | Check your internet connection and Git setup, then re-run `cmake --preset x64-debug`. |
| Blank window / missing assets | Ensure `ASSETS_PATH` is defined correctly during build (points to `ChessEngine/assets/`). |
| Build fails on Linux/macOS | Verify that your compiler supports C++23 (`g++ --version` or `clang --version`). |

---

## Design Notes

The chessboard is internally represented as a **1D array of 64 squares**, which improves **cache locality** and simplifies indexing.  
Move generation and validation are implemented in `Core`, using simple bitwise operations for speed.  

Rendering is decoupled from logic and handled by **Dear ImGui** layered on GLFW/OpenGL, enabling smooth frame updates and a responsive immediate-mode UI.
The codebase is intentionally kept **modular and dependency-free**, making it suitable for experimentation, AI integration, or future porting to 3D.

---

## Contributing

Contributions and discussions are welcome — whether it’s about engine architecture, optimization, or UX improvements.  
Please open an issue or a pull request with clear context and comments.  

If you’re experimenting with engine logic, prefer **isolated modules** and **consistent naming conventions** (`PascalCase` for types, `snake_case` for functions and variables).

---

## Build Summary (TL;DR)

```bash
git clone https://github.com/yourname/ChessEngine.git
cd ChessEngine
mkdir build && cd build
cmake .. && cmake --build .
```

Run the binary:
```bash
./build/ChessEngine
```
