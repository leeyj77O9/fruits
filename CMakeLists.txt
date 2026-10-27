cmake_minimum_required(VERSION 3.20)

project(fruits C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SDL3 CONFIG REQUIRED)
find_package(SDL3_ttf CONFIG REQUIRED)

add_executable(fruits
    src/main.c
    src/game/fruit.c
    src/game/state.c
    src/util/vec2.c
    src/util/type.c
    src/util/rgba.c
    src/game/renderer.c
    src/game/frame_clock.c
    src/game/collider.c
    src/util/rect.c
)

target_link_libraries(fruits PRIVATE SDL3::SDL3 SDL3_ttf::SDL3_ttf)
