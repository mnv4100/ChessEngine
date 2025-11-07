cmake_minimum_required(VERSION 3.10)
project(ChessEngineProject)

# Politique pour MSVC
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
endif()

# Forcer la runtime dynamique (/MD ou /MDd)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL" CACHE STRING "" FORCE)

# Fetch raylib
include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
    GIT_TAG "master"
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(raylib)

# Add the real project
add_subdirectory(ChessEngine)
