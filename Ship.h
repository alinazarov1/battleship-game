#pragma once
#ifndef SHIP_H
#define SHIP_H

#include <SFML/Graphics.hpp>
#include <vector>
#include "Common.h"

class Ship {
public:
    int size;
    int x, y;
    bool horizontal;
    bool placed;
    bool sunk;
    std::vector<bool> hitParts;
    ShipType type;
    sf::Color shipColor;

    Ship(int s);
    bool isPartHit(int partIndex) const;
    void hitPart(int partIndex);
    void checkIfSunk();
};

#endif
