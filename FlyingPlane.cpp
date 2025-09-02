#include "FlyingPlane.h"

FlyingPlane::FlyingPlane() :
    startPos(0, 0), endPos(0, 0), currentPos(0, 0), progress(0),
    active(false), isPlayerPlane(false), speed(800.0f), rotation(0) {
}

void FlyingPlane::start(sf::Vector2f start, sf::Vector2f end, bool player) {
    startPos = start;
    endPos = end;
    currentPos = start;
    progress = 0;
    active = true;
    isPlayerPlane = player;
    speed = 800.0f;

    sf::Vector2f direction = endPos - startPos;
    rotation = atan2(direction.y, direction.x) * 180.0f / 3.14159f;
}

void FlyingPlane::update(float deltaTime) {
    if (!active) return;

    progress += speed * deltaTime /
        sqrt(pow(endPos.x - startPos.x, 2) + pow(endPos.y - startPos.y, 2));

    if (progress >= 1.0f) {
        progress = 1.0f;
        active = false;
    }

    float smoothProgress = progress * progress * (3.0f - 2.0f * progress);
    currentPos = startPos + smoothProgress * (endPos - startPos);
}

void FlyingPlane::draw(sf::RenderWindow& window) {
    if (!active) return;

    sf::RectangleShape plane(sf::Vector2f(30, 8));
    plane.setFillColor(isPlayerPlane ? sf::Color(100, 150, 255) : sf::Color(255, 100, 100));
    plane.setOrigin(15, 4);
    plane.setPosition(currentPos);
    plane.setRotation(rotation);
    window.draw(plane);

    sf::RectangleShape wing1(sf::Vector2f(12, 3));
    wing1.setFillColor(plane.getFillColor());
    wing1.setOrigin(6, 1.5f);
    wing1.setPosition(currentPos.x - 8 * cos(rotation * 3.14159f / 180.0f),
        currentPos.y - 8 * sin(rotation * 3.14159f / 180.0f));
    wing1.setRotation(rotation + 90);
    window.draw(wing1);

    sf::RectangleShape wing2(sf::Vector2f(12, 3));
    wing2.setFillColor(plane.getFillColor());
    wing2.setOrigin(6, 1.5f);
    wing2.setPosition(currentPos.x - 8 * cos(rotation * 3.14159f / 180.0f),
        currentPos.y - 8 * sin(rotation * 3.14159f / 180.0f));
    wing2.setRotation(rotation - 90);
    window.draw(wing2);

    sf::CircleShape propeller(3);
    propeller.setFillColor(sf::Color(200, 200, 200, 150));
    propeller.setOrigin(3, 3);
    propeller.setPosition(currentPos.x + 15 * cos(rotation * 3.14159f / 180.0f),
        currentPos.y + 15 * sin(rotation * 3.14159f / 180.0f));
    window.draw(propeller);
}
