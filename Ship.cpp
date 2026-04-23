#include "Ship.h"

Ship::Ship(int s) : size(s), x(0), y(0), horizontal(true), placed(false), sunk(false),
type(ShipType::SUBMARINE), shipColor(sf::Color(60, 80, 120)) {
    hitParts.resize(s, false);

    switch (s) {
    case 4:
        type = ShipType::BATTLESHIP;
        shipColor = sf::Color(120, 120, 120);
        break;
    case 3:
        type = ShipType::CRUISER;
        shipColor = sf::Color(100, 100, 150);
        break;
    case 2:
        type = ShipType::DESTROYER;
        shipColor = sf::Color(80, 120, 80);
        break;
    case 1:
        type = ShipType::SUBMARINE;
        shipColor = sf::Color(60, 80, 120);
        break;
    }
}

bool Ship::isPartHit(int partIndex) const {
    return partIndex >= 0 && partIndex < static_cast<int>(hitParts.size()) && hitParts[partIndex];
}

void Ship::hitPart(int partIndex) {
    if (partIndex >= 0 && partIndex < static_cast<int>(hitParts.size())) {
        hitParts[partIndex] = true;
        checkIfSunk();
    }
}

void Ship::checkIfSunk() {
    sunk = true;
    for (bool hit : hitParts) {
        if (!hit) {
            sunk = false;
            break;
        }
    }
}
