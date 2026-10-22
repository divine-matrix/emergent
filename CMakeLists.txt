cmake_minimum_required(VERSION 3.20)
project(EmergentGarden VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SDL3 globally
find_package(SDL3 REQUIRED)

# Point straight to the main source directory
add_subdirectory(src)
