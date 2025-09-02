#include "Effect.h"

AnimatedEffect::AnimatedEffect() : lifetime(0), maxLifetime(1.0f), scale(1.0f), active(false) {}

void AnimatedEffect::start(sf::Vector2f pos, sf::Color col, float life) {
    position = pos;
    color = col;
    maxLifetime = life;
    lifetime = life;
    scale = 1.0f;
    active = true;
}

void AnimatedEffect::update(float deltaTime) {
    if (!active) return;

    lifetime -= deltaTime;
    if (lifetime <= 0) {
        active = false;
        return;
    }

    float progress = 1.0f - (lifetime / maxLifetime);
    scale = 1.0f + progress * 2.0f;
    color.a = static_cast<sf::Uint8>(255 * (lifetime / maxLifetime));
}

WaveEffect::WaveEffect() : radius(0), maxRadius(100), thickness(3), active(false) {}

void WaveEffect::start(sf::Vector2f pos, sf::Color col, float maxRad) {
    center = pos;
    color = col;
    maxRadius = maxRad;
    radius = 0;
    active = true;
}

void WaveEffect::update(float deltaTime) {
    if (!active) return;

    radius += 150.0f * deltaTime;
    if (radius >= maxRadius) {
        active = false;
        return;
    }

    float progress = radius / maxRadius;
    color.a = static_cast<sf::Uint8>(255 * (1.0f - progress));
}

void WaveEffect::draw(sf::RenderWindow& window) {
    if (!active) return;

    sf::CircleShape circle(radius);
    circle.setFillColor(sf::Color::Transparent);
    circle.setOutlineColor(color);
    circle.setOutlineThickness(thickness);
    circle.setPosition(center.x - radius, center.y - radius);
    window.draw(circle);
}
