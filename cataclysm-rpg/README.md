# Cataclysm RPG

Post-apocalyptic survival RPG inspired by CDDA.

## Note: SDL2_mixer Temporary Workaround
**Current Status:** This project currently uses SDL2_mixer instead of SDL3_mixer due to SDL3_mixer's late release (version 3.1.0 requiring SDL3 ≥ 3.3.0, while SDL3 3.2.22 is currently available). The SDL2_mixer API is nearly identical to SDL3_mixer, providing the same audio functionality with better stability. This will be migrated back to SDL3_mixer once compatibility issues are resolved.

## Setup on Arch Linux
sudo pacman -S sdl3 sdl3_image sdl2_mixer cmake nlohmann-json base-devel python-pipenv
pip install pillow  # For atlas gen

cd tools
pipenv install
./build_datasets.sh

mkdir build && cd build
cmake ..
make
./cataclysm-rpg
