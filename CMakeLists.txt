cmake_minimum_required(VERSION 3.16)

project(chip8 C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(cpu STATIC src/cpu.c)
target_include_directories(cpu PUBLIC include)
target_compile_options(cpu PRIVATE -Wall -Wextra)

add_library(rom STATIC src/rom.c)
target_include_directories(rom PUBLIC include)
target_compile_options(rom PRIVATE -Wall -Wextra)

add_executable(chip8 src/main.c)
target_link_libraries(chip8 PRIVATE cpu rom)
target_compile_options(chip8 PRIVATE -Wall -Wextra)

find_package(SDL2 REQUIRED)
