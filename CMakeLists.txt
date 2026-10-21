cmake_minimum_required(VERSION 3.20)

project(fruits C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SDL3 CONFIG REQUIRED)

add_executable(fruits
    src/main.c
)

target_link_libraries(fruits PRIVATE SDL3::SDL3)
