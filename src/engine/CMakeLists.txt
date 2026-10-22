# Define the target library name
add_library(RaptorEngine
    base.cpp
    base.h
    render.cpp
    render.h
)

# Crucial: Export this directory's path as a PUBLIC include directory.
# This tells CMake that anyone linking to "MyEngine" can find its headers instantly.

target_link_libraries(RaptorEngine PUBLIC
    SDL3::SDL3
    imgui::imgui
)
target_include_directories(RaptorEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
