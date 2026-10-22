# 1. Dive deeper into the src subfolders first to build the library targets
add_subdirectory(engine)

# 2. Define your executable using the main.cpp file located here
add_executable(EmergentGarden
    main.cpp
)

# 3. Link the subdirectory library and SDL3 directly into your app
target_link_libraries(EmergentGarden PRIVATE RaptorEngine SDL3::SDL3)
