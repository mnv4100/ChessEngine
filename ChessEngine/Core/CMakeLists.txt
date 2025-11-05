# Core library: sources live one level up from this CMakeLists
add_library(CoreLib
        Core.cpp
        Core.h)

# Expose include path where headers actually live
target_include_directories(CoreLib
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..
)