# Local CMakeLists for the ChessEngine target
cmake_minimum_required(VERSION 3.10)
project(ChessEngine VERSION 0.1)

# Global C++ standard for this subproject
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Sub-libraries (compile their own sources)
add_subdirectory(Core)
add_subdirectory(Io)
add_subdirectory(Controller)

# Executable sources: only files that are NOT compiled inside the sub-libraries
add_executable(${PROJECT_NAME}
        main.cpp
        definition.h
)

# Ensure include path for headers in this folder
target_include_directories(${PROJECT_NAME}
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

# Define assets path for runtime
target_compile_definitions(${PROJECT_NAME} PUBLIC
        ASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/")

# Link the internal libraries and raylib (fetched in the root CMake)
target_link_libraries(${PROJECT_NAME}
        PRIVATE CoreLib
        PRIVATE IOLib
        PRIVATE ControllerLib
        PRIVATE raylib
)


