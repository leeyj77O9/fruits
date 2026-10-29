cmake_minimum_required(VERSION 3.21)

project(fruits C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SDL3 CONFIG REQUIRED)
find_package(SDL3_image CONFIG REQUIRED)
find_package(SDL3_ttf CONFIG REQUIRED)

add_executable(fruits
    src/main.c
    src/game/collider.c
    src/game/effect.c
    src/game/frame_clock.c
    src/game/fruit.c
    src/game/game.c
    src/game/renderer.c
    src/game/state.c
    src/game/text.c
    src/util/vec2.c
)

target_include_directories(fruits PRIVATE src)
target_link_libraries(fruits PRIVATE SDL3::SDL3 SDL3_image::SDL3_image SDL3_ttf::SDL3_ttf)

add_custom_command(TARGET fruits POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/asset"
        "$<TARGET_FILE_DIR:fruits>/asset"
    VERBATIM
)

if(MSVC)
    target_compile_options(fruits PRIVATE /utf-8)
endif()

if(WIN32)
    set_target_properties(fruits PROPERTIES WIN32_EXECUTABLE TRUE)

    add_custom_command(TARGET fruits POST_BUILD
        COMMAND ${CMAKE_COMMAND}
            "-DDLLS=$<TARGET_RUNTIME_DLLS:fruits>"
            "-DDESTINATION=$<TARGET_FILE_DIR:fruits>"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/copy_runtime_dlls.cmake"
        VERBATIM
    )
endif()
