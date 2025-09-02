#pragma once
#ifndef FLYINGPLANE_H
#define FLYINGPLANE_H

#include <SFML/Graphics.hpp>
#include <cmath>

class FlyingPlane {
public:
    sf::Vector2f startPos, endPos, currentPos;
    float progress;
    bool active;
    bool isPlayerPlane;
    float speed;
    float rotation;

    FlyingPlane();
    void start(sf::Vector2f start, sf::Vector2f end, bool player);
    void update(float deltaTime);
    void draw(sf::RenderWindow& window);
};

#endif
