cmake_minimum_required(VERSION 3.16)

project(chip8 C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SDL2 REQUIRED)
enable_testing()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

option(CHIP8_SANITIZE "Build with AddressSanitizer and UBSan" OFF)

add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wconversion)

if(CHIP8_SANITIZE)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

add_library(cpu STATIC src/cpu.c src/opcodes.c src/keypad.c)
target_include_directories(cpu PUBLIC include)

add_library(rom STATIC src/rom.c)
target_include_directories(rom PUBLIC include)

add_library(display STATIC src/display.c)
target_include_directories(display PUBLIC include)
target_link_libraries(display PUBLIC SDL2::SDL2 cpu)

add_executable(chip8 src/main.c)
target_link_libraries(chip8 PRIVATE cpu rom display)

add_executable(chip8_tests
    tests/main.c
    tests/harness/harness.c
    tests/unit/opcodes.c
    tests/unit/cpu.c
    tests/unit/keypad.c
    tests/unit/display.c
)
target_include_directories(chip8_tests PRIVATE tests/harness)
target_link_libraries(chip8_tests PRIVATE cpu)
add_test(NAME chip8_tests COMMAND chip8_tests)
