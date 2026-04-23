#pragma once
#ifndef COMMON_H
#define COMMON_H

enum class ArsenalType {
    RADAR,
    AIRPLANE,
    HELICOPTER
};

enum class GameState {
    MENU,
    RULES,
    CREDITS,
    SHIP_PLACEMENT,
    BATTLE,
    GAME_OVER
};

enum class CellState {
    EMPTY = 0,
    SHIP = 1,
    SHIP_AREA = 2,
    HIT = 3,
    MISS = 4,
    SUNK = 5
};

enum class ShipType {
    BATTLESHIP = 4,
    CRUISER = 3,
    DESTROYER = 2,
    SUBMARINE = 1
};

constexpr int BOARD_SIZE = 10;
constexpr int CELL_SIZE = 45;
constexpr int BOARD_OFFSET_X = 50;
constexpr int BOARD_OFFSET_Y = 180;
constexpr int COMPUTER_BOARD_OFFSET_X = 680;
constexpr float ANIMATION_SPEED = 5.0f;

#endif
