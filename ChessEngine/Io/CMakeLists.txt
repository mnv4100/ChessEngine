add_library(IOLib
        Io.cpp
        Io.h
)

# Headers live one level up from here
target_include_directories(IOLib
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..
)


# Depends on Core and raylib
# raylib must be PUBLIC because Io.h includes raylib.h and is consumed by other targets
# This exposes raylib's include directories (and link) to dependents like ControllerLib
target_link_libraries(IOLib
        PRIVATE CoreLib
        PUBLIC raylib
)

# Define assets path for runtime
target_compile_definitions(IOLib PRIVATE
        ASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../assets/")