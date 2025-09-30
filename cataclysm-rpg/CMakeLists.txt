cmake_minimum_required(VERSION 3.20)
project(CataclysmRPG)

set(CMAKE_CXX_STANDARD 20)

find_package(SDL3 REQUIRED)
find_package(SDL3_image REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(nlohmann_json REQUIRED)

add_executable(cataclysm-rpg src/main.cpp
    src/engine/renderer.cpp src/engine/renderer.h
    src/engine/input.cpp src/engine/input.h
    src/engine/audio.cpp src/engine/audio.h
    src/engine/particles.cpp src/engine/particles.h
    src/engine/lighting.cpp src/engine/lighting.h
    src/engine/utils/log.cpp src/engine/utils/log.h
    src/game/world.cpp src/game/world.h
    src/game/actor.cpp src/game/actor.h
    src/game/items.cpp src/game/items.h
    src/game/ui.cpp src/game/ui.h
    src/game/tiles.cpp src/game/tiles.h
    src/game/spell.h
)

target_link_libraries(cataclysm-rpg SDL3::SDL3 SDL2_mixer::SDL2_mixer nlohmann_json::nlohmann_json SDL3_image::SDL3_image)
