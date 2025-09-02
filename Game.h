#pragma once
#include <SFML/Graphics.hpp>

#include <SFML/OpenGL.hpp>
#include <GL/gl.h>
#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <cmath>
#include "Common.h"
#include "Ship.h"
#include "FlyingPlane.h"
#include "Effect.h"
#include "Arsenal.h"



class Game {
private:
    // Game state variables
    sf::RenderWindow window;
    sf::Font font;
    sf::Font titleFont;
    GameState currentState;
    sf::Clock clock;
    float deltaTime;

    // Game boards and ships
    int playerBoard[BOARD_SIZE][BOARD_SIZE];
    int computerBoard[BOARD_SIZE][BOARD_SIZE];
    std::vector<Ship> playerShips;
    std::vector<Ship> computerShips;
    sf::Texture shipTextures[4];
    int currentShipIndex;

    // Gameplay variables
    int playerScore;
    int computerScore;
    bool playerTurn;
    std::vector<std::pair<int, int>> targetQueue;

    // UI variables
    sf::Vector2i mousePos;
    sf::Vector2i hoveredCell;
    bool showingComputerShips;

    // Effects
    std::vector<AnimatedEffect> effects;
    std::vector<WaveEffect> waves;
    std::vector<FlyingPlane> planes;
    float backgroundWaveOffset;
    float waterAnimationTime;

    // Colors
    sf::Color deepBlue = sf::Color(20, 50, 100);
    sf::Color lightBlue = sf::Color(50, 150, 200);
    sf::Color darkGreen = sf::Color(34, 139, 34);
    sf::Color lightGreen = sf::Color(144, 238, 144);
    sf::Color shipGray = sf::Color(105, 105, 105);
    sf::Color hitRed = sf::Color(220, 20, 60);
    sf::Color missWhite = sf::Color(255, 255, 255);
    sf::Color sunkDarkRed = sf::Color(139, 0, 0);
    sf::Color waterBlue1 = sf::Color(30, 80, 150);
    sf::Color waterBlue2 = sf::Color(40, 100, 180);
    sf::Color penBlue = sf::Color(40, 40, 180);
    bool texturesLoaded = false;

    // Arsenal system
    std::vector<Arsenal> arsenals;
    bool selectingArsenal;
    ArsenalType selectedArsenal;
    bool attackingWithArsenal;
    sf::Vector2i arsenalTarget;
    float arsenalAnimationProgress;

    // Private methods
    void initArsenals();
    void drawArsenalMenu();
    void draw3DAnimation(ArsenalType type, sf::Vector2f target);
    void processArsenalAttack();
    void initializeBoards();
    void initializeShips();
    void placeComputerShips();
    void updateHoveredCell();
    bool canPlaceShip(int board[BOARD_SIZE][BOARD_SIZE], int x, int y, int size, bool horizontal);
    void placeShip(int board[BOARD_SIZE][BOARD_SIZE], int x, int y, int size, bool horizontal);
    void markShipArea(int board[BOARD_SIZE][BOARD_SIZE], int x, int y, int size, bool horizontal);
    void checkAndMarkSunkShip(int board[BOARD_SIZE][BOARD_SIZE], std::vector<Ship>& ships, int hitX, int hitY);
    void addAdjacentTargets(int x, int y);
    void addEffect(sf::Vector2f pos, sf::Color color, float lifetime);
    void addWave(sf::Vector2f pos, sf::Color color, float maxRadius);
    void drawDoodleShip(int x, int y, int size, bool horizontal, ShipType type, bool sunk);
    void drawBoard(int offsetX, int offsetY, int board[BOARD_SIZE][BOARD_SIZE], bool showShips);
    std::string getCurrentShipName();
    void restartGame();

public:
    Game();
    void run();
    void handleEvents();
    void handleKeyPress(sf::Keyboard::Key key);
    void handleMouseClick(int x, int y);
    void handleShipPlacement(int x, int y);
    void handlePlayerShot(int x, int y);
    void update();
    void computerShoot();
    void render();
    void renderMenu();
    void renderRules();
    void renderCredits();
    void renderShipPlacement();
    void renderBattle();
    void renderGameOver();
    void drawAnimatedBackground();
    void drawEffects();
    void drawPlanes();
    void addFlyingPlane(sf::Vector2f start, sf::Vector2f end, bool isPlayerPlane);
};