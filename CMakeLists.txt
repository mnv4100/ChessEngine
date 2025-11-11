cmake_minimum_required(VERSION 3.10)
project(ChessEngineProject)

# Politique pour MSVC
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
endif()

# Prefer GLVND when available for modern OpenGL loaders
if (POLICY CMP0072)
  cmake_policy(SET CMP0072 NEW)
endif()

set(OpenGL_GL_PREFERENCE GLVND)

# Forcer la runtime dynamique (/MD ou /MDd)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL" CACHE STRING "" FORCE)

# Fetch dependencies for Dear ImGui rendering stack
include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)

# GLFW configuration (windowing + input)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
set(GLFW_LIBRARY_TYPE "STATIC" CACHE STRING "" FORCE)

if (WIN32)
  set(GLFW_BUILD_WIN32 ON CACHE BOOL "" FORCE)
  set(GLFW_BUILD_COCOA OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_X11 OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)
elseif(APPLE)
  set(GLFW_BUILD_WIN32 OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_COCOA ON CACHE BOOL "" FORCE)
  set(GLFW_BUILD_X11 OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)
else()
  set(GLFW_BUILD_WIN32 OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_COCOA OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_X11 ON CACHE BOOL "" FORCE)
  set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)
endif()

FetchContent_Declare(
    glfw
    GIT_REPOSITORY "https://github.com/glfw/glfw.git"
    GIT_TAG "3.4"
    GIT_PROGRESS TRUE
)

# glad OpenGL loader
set(GLAD_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    glad
    GIT_REPOSITORY "https://github.com/Dav1dde/glad.git"
    GIT_TAG "v2.0.8"
    GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(glfw glad)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY "https://github.com/ocornut/imgui.git"
    GIT_TAG "v1.92.4"
    GIT_PROGRESS TRUE
)

FetchContent_GetProperties(imgui)
if (NOT imgui_POPULATED)
    FetchContent_Populate(imgui)
endif()

find_package(OpenGL REQUIRED)

set(IMGUI_DIR ${imgui_SOURCE_DIR})
add_library(imgui STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui
    PUBLIC
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
)

target_link_libraries(imgui
    PUBLIC
        glfw
        glad
        OpenGL::GL
)

target_compile_definitions(imgui
    PUBLIC
        IMGUI_IMPL_OPENGL_LOADER_GLAD
)

# Add the real project
add_subdirectory(ChessEngine)
