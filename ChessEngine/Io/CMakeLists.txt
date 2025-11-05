add_library(IOLib
        Io.cpp
        Io.h
)

# Headers live one level up from here
target_include_directories(IOLib
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..
)


# Depends on Core and raylib
target_link_libraries(IOLib
        PRIVATE CoreLib
        PRIVATE raylib
)

# Define assets path for runtime
target_compile_definitions(IOLib PRIVATE
        ASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../assets/")