cmake_minimum_required (VERSION 3.10)

# Define the project
project(chip8)

# GoogleTest requires at least C++11
set(CMAKE_CXX_STANDARD 11)

find_package(SDL REQUIRED)
include_directories(chip8 ${SDL_INCLUDE_DIRS})

# Add definition for the cmake-tutorial binary
add_executable(chip8 src/main.cpp src/chip8.cpp)
target_link_libraries(chip8 ${SDL_LIBRARIES})
