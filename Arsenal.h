#pragma once
#ifndef ARSENAL_H
#define ARSENAL_H

#include <SFML/Graphics.hpp>
#include "Common.h"

class Arsenal {
public:
    ArsenalType type;
    int count;
    sf::Texture icon;

    Arsenal(ArsenalType t, int c);
};

#endif // ARSENAL_H
