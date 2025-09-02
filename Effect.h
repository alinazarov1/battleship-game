#pragma once
#ifndef EFFECT_H
#define EFFECT_H

#include <SFML/Graphics.hpp>

class AnimatedEffect {
public:
    sf::Vector2f position;
    float lifetime;
    float maxLifetime;
    sf::Color color;
    float scale;
    bool active;

    AnimatedEffect();
    void start(sf::Vector2f pos, sf::Color col, float life);
    void update(float deltaTime);
};

class WaveEffect {
public:
    sf::Vector2f center;
    float radius;
    float maxRadius;
    float thickness;
    sf::Color color;
    bool active;

    WaveEffect();
    void start(sf::Vector2f pos, sf::Color col, float maxRad);
    void update(float deltaTime);
    void draw(sf::RenderWindow& window);
};

#endif
