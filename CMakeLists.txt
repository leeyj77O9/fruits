cmake_minimum_required(VERSION 3.20)

project(fruits C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SDL3 CONFIG REQUIRED)
find_package(SDL3_ttf CONFIG REQUIRED)

add_executable(fruits
    src/main.c
    src/game/collider.c
    src/game/frame_clock.c
    src/game/fruit.c
    src/game/game.c
    src/game/renderer.c
    src/game/state.c
    src/game/text.c
    src/util/vec2.c
)

target_include_directories(fruits PRIVATE src)
target_link_libraries(fruits PRIVATE SDL3::SDL3 SDL3_ttf::SDL3_ttf)
