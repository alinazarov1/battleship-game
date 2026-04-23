# Battleship (SFML)

A desktop Battleship game written in C++ using SFML.

## Features

- Ship placement phase with rotation
- Battle phase with hit/miss/sunk states
- Animated effects and flying projectiles
- Special arsenal attacks (radar, air strike, helicopter)
- Menu, rules, credits, and game over screens

## Prerequisites

- CMake 3.16+
- A C++17 compiler

SFML setup is automatic by default. If SFML is not already installed, CMake will fetch it during configure.
On Linux, SFML still needs system graphics development packages.

### Ubuntu/Debian

```bash
sudo apt update
sudo apt install -y build-essential cmake libgl1-mesa-dev xorg-dev
```

Optional (faster configure, use system package instead of fetching):

```bash
sudo apt install -y libsfml-dev
```

## Build

```bash
cmake -S . -B build
cmake --build build -j
```

If you want to disable auto-fetch and require a system-installed SFML:

```bash
cmake -S . -B build -DBATTLESHIP_FETCH_SFML=OFF
```

## Run

```bash
./build/Battleship
```

## Controls

- `1` Start game
- `2` Rules
- `3` Credits
- `4` Exit
- `R` Rotate ship (placement) / restart (game over)
- `D` Toggle enemy-ship debug view (battle)
- `Esc` Return to menu
- Mouse click to place ships / fire

## Assets

The game expects assets in an `assets/` directory next to the source tree. On build, this folder is copied beside the executable if present.
