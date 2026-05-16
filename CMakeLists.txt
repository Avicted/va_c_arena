cmake_minimum_required(VERSION 3.16)
project(va_c_arena C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-ggdb -O0 -Wall -Wextra -Werror -Wpedantic)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(example src/example.c)
target_link_libraries(example m)

add_executable(example_aligned src/example_aligned.c)
target_link_libraries(example_aligned m)

add_executable(tests tests/test_va_arena.c)
target_link_libraries(tests m)
