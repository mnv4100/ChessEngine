cmake_minimum_required(VERSION 3.16)
project(ChessEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(chess_engine
    src/board.cpp
    src/game.cpp
    src/notation.cpp
    src/console_ui.cpp
)

target_include_directories(chess_engine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(chess_engine PUBLIC cxx_std_20)

add_executable(chess_cli src/main.cpp)
target_link_libraries(chess_cli PRIVATE chess_engine)

find_package(SDL3 CONFIG REQUIRED)
find_package(SDL3_image CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

add_executable(chess_sdl
    src/sdl_main.cpp
    src/sdl_ui.cpp
)

target_link_libraries(chess_sdl PRIVATE chess_engine SDL3_image::SDL3_image SDL3::SDL3 imgui::imgui)
target_compile_definitions(chess_sdl PRIVATE CHESS_ASSETS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets")


if (MSVC)
    target_compile_options(chess_engine PRIVATE /W4 /permissive-)
    target_compile_options(chess_cli PRIVATE /W4 /permissive-)
    target_compile_options(chess_sdl PRIVATE /W4 /permissive-)
else()
    target_compile_options(chess_engine PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(chess_cli PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(chess_sdl PRIVATE -Wall -Wextra -Wpedantic)
endif()
