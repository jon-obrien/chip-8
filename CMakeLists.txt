project(chip8 C)

cmake_minimum_required(VERSION 3.16)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(chip8 src/main.c)

find_package(SDL2 REQUIRED)
target_link_libraries(chip8 PRIVATE SDL2::SDL2)

target_compile_options(chip8 PRIVATE -Wall -Wextra)
