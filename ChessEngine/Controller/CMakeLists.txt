# Build Controller as a real library so its sources are compiled and linked
add_library(ControllerLib STATIC
        Controller.cpp
)

# Public include directory for dependents
target_include_directories(ControllerLib
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link dependencies (Io exposes rendering/backend stack)
target_link_libraries(ControllerLib
        PUBLIC CoreLib
        PUBLIC IOLib
)
