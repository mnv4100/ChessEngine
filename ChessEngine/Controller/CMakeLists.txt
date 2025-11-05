# Controller library currently has no source files; expose as INTERFACE for dependencies
add_library(ControllerLib INTERFACE
        Controller.cpp
        Controller.h)

# Expose include path to dependents
target_include_directories(ControllerLib
        INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link dependencies transitively
target_link_libraries(ControllerLib
        INTERFACE CoreLib
        INTERFACE IOLib
)
