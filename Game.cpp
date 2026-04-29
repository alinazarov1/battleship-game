#include "Game.h"

namespace {
bool loadFirstAvailableFont(sf::Font& font, const std::vector<std::string>& fontPaths) {
    for (const auto& fontPath : fontPaths) {
        if (font.loadFromFile(fontPath)) {
            return true;
        }
    }
    return false;
}
}

Game::Game() : window(sf::VideoMode(1200, 900), "Battleship"),
currentState(GameState::MENU),
currentShipIndex(0),
playerScore(0),
computerScore(0),
playerTurn(true),
hoveredCell(-1, -1),
showingComputerShips(false),
backgroundWaveOffset(0),
selectingArsenal(false),
selectedArsenal(ArsenalType::RADAR),
attackingWithArsenal(false),
arsenalAnimationProgress(0),
waterAnimationTime(0) {

    srand(static_cast<unsigned int>(time(nullptr)));

    // Load fonts
    if (!loadFirstAvailableFont(font, {
            "assets/arial.ttf",
            "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
            "/usr/share/fonts/truetype/liberation2/LiberationSans-Regular.ttf"
        })) {
        std::cout << "Warning: Could not load a font file, text may not render\n";
    }
    titleFont = font;

    // Initialize effects and animations
    effects.resize(15);
    waves.resize(8);
    planes.resize(5);
    texturesLoaded = true;
    if (!shipTextures[0].loadFromFile("assets/ship1.png")) {
        std::cerr << "Failed to load ship1.png\n";
        texturesLoaded = false;
    }
    if (!shipTextures[1].loadFromFile("assets/ship2.png")) {
        std::cerr << "Failed to load ship2.png\n";
        texturesLoaded = false;
    }
    if (!shipTextures[2].loadFromFile("assets/ship3.png")) {
        std::cerr << "Failed to load ship3.png\n";
        texturesLoaded = false;
    }
    if (!shipTextures[3].loadFromFile("assets/ship4.png")) {
        std::cerr << "Failed to load ship4.png\n";
        texturesLoaded = false;
    }
    initializeBoards();
    initializeShips();
    placeComputerShips();
    initArsenals();

    window.setVerticalSyncEnabled(true);
}

void Game::run() {
    while (window.isOpen()) {
        deltaTime = clock.restart().asSeconds();
        backgroundWaveOffset += deltaTime * 20.0f;
        waterAnimationTime += deltaTime;

        handleEvents();
        update();
        render();
    }
}

void Game::handleEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window.close();
        }

        if (event.type == sf::Event::MouseMoved) {
            mousePos = sf::Mouse::getPosition(window);
            updateHoveredCell();
        }

        if (event.type == sf::Event::MouseButtonPressed) {
            handleMouseClick(event.mouseButton.x, event.mouseButton.y);
        }

        if (event.type == sf::Event::KeyPressed) {
            handleKeyPress(event.key.code);
        }
    }
}

void Game::handleKeyPress(sf::Keyboard::Key key) {
    switch (currentState) {
    case GameState::MENU:
        if (key == sf::Keyboard::Num1) currentState = GameState::SHIP_PLACEMENT;
        if (key == sf::Keyboard::Num2) currentState = GameState::RULES;
        if (key == sf::Keyboard::Num3) currentState = GameState::CREDITS;
        if (key == sf::Keyboard::Num4) window.close();
        break;

    case GameState::RULES:
    case GameState::CREDITS:
        if (key == sf::Keyboard::Escape) currentState = GameState::MENU;
        break;

    case GameState::SHIP_PLACEMENT:
        if (key == sf::Keyboard::R && currentShipIndex < playerShips.size()) {
            playerShips[currentShipIndex].horizontal = !playerShips[currentShipIndex].horizontal;
        }
        if (key == sf::Keyboard::Escape) currentState = GameState::MENU;
        break;

    case GameState::BATTLE:
        if (key == sf::Keyboard::D) showingComputerShips = !showingComputerShips;
        if (key == sf::Keyboard::Escape) currentState = GameState::MENU;
        break;

    case GameState::GAME_OVER:
        if (key == sf::Keyboard::R) restartGame();
        if (key == sf::Keyboard::Escape) currentState = GameState::MENU;
        break;
    }
}

void Game::handleMouseClick(int x, int y) {
    switch (currentState) {
    case GameState::SHIP_PLACEMENT:
        handleShipPlacement(x, y);
        break;

    case GameState::BATTLE:
        // Check if clicked on the special attack button (top-left area)
        if (x >= 50 && x <= 250 && y >= 50 && y <= 90) {
            selectingArsenal = true;
            break;
        }

        if (selectingArsenal) {
            // Check arsenal selection menu clicks
            float buttonY = window.getSize().y / 2 - 20;
            for (size_t i = 0; i < arsenals.size(); i++) {
                if (arsenals[i].count > 0) {
                    if (x > window.getSize().x / 2 - 100 && x < window.getSize().x / 2 + 100 &&
                        y > buttonY && y < buttonY + 30) {
                        selectedArsenal = arsenals[i].type;
                        selectingArsenal = false;
                        attackingWithArsenal = true;
                        break;
                    }
                    buttonY += 40;
                }
            }
            // Clicked outside menu - cancel selection
            selectingArsenal = false;
            break;
        }

        if (attackingWithArsenal) {
            // Handle arsenal attack targeting
            int boardX = (x - COMPUTER_BOARD_OFFSET_X) / CELL_SIZE;
            int boardY = (y - BOARD_OFFSET_Y) / CELL_SIZE;

            if (boardX >= 0 && boardX < BOARD_SIZE && boardY >= 0 && boardY < BOARD_SIZE) {
                arsenalTarget = sf::Vector2i(boardX, boardY);
                // Animation will be handled in the update/render functions
            }
            break;
        }

        if (playerTurn && !attackingWithArsenal) {
            handlePlayerShot(x, y);
        }
        break;
    }
}

void Game::handleShipPlacement(int x, int y) {
    if (currentShipIndex >= playerShips.size()) return;

    int boardX = (x - BOARD_OFFSET_X) / CELL_SIZE;
    int boardY = (y - BOARD_OFFSET_Y) / CELL_SIZE;

    if (boardX >= 0 && boardX < BOARD_SIZE && boardY >= 0 && boardY < BOARD_SIZE) {
        Ship& ship = playerShips[currentShipIndex];

        if (canPlaceShip(playerBoard, boardX, boardY, ship.size, ship.horizontal)) {
            placeShip(playerBoard, boardX, boardY, ship.size, ship.horizontal);
            ship.x = boardX;
            ship.y = boardY;
            ship.placed = true;
            currentShipIndex++;

            // Add placement effect
            addEffect(sf::Vector2f(x, y), lightGreen, 0.8f);

            if (currentShipIndex >= playerShips.size()) {
                currentState = GameState::BATTLE;
            }
        }
    }
}

void Game::handlePlayerShot(int x, int y) {
    int boardX = (x - COMPUTER_BOARD_OFFSET_X) / CELL_SIZE;
    int boardY = (y - BOARD_OFFSET_Y) / CELL_SIZE;

    if (boardX >= 0 && boardX < BOARD_SIZE && boardY >= 0 && boardY < BOARD_SIZE) {
        if (computerBoard[boardY][boardX] != static_cast<int>(CellState::HIT) &&
            computerBoard[boardY][boardX] != static_cast<int>(CellState::MISS) &&
            computerBoard[boardY][boardX] != static_cast<int>(CellState::SUNK)) {

            sf::Vector2f shotPos(COMPUTER_BOARD_OFFSET_X + boardX * CELL_SIZE + CELL_SIZE / 2,
                BOARD_OFFSET_Y + boardY * CELL_SIZE + CELL_SIZE / 2);

            // Add flying plane animation
            sf::Vector2f planeStart(BOARD_OFFSET_X + BOARD_SIZE * CELL_SIZE + 50,
                BOARD_OFFSET_Y + BOARD_SIZE * CELL_SIZE / 2);
            addFlyingPlane(planeStart, shotPos, true);

            // Delay the actual hit registration to sync with plane arrival
            if (computerBoard[boardY][boardX] == static_cast<int>(CellState::SHIP)) {
                computerBoard[boardY][boardX] = static_cast<int>(CellState::HIT);
                playerScore++;

                // Hit effect
                addEffect(shotPos, hitRed, 1.2f);
                addWave(shotPos, hitRed, 60.0f);

                // Check if ship is sunk
                checkAndMarkSunkShip(computerBoard, computerShips, boardX, boardY);
            }
            else {
                computerBoard[boardY][boardX] = static_cast<int>(CellState::MISS);

                // Miss effect
                addEffect(shotPos, missWhite, 0.8f);
                addWave(shotPos, lightBlue, 40.0f);
            }

            playerTurn = false;

            if (playerScore >= 20) {
                currentState = GameState::GAME_OVER;
            }
        }
    }
}

void Game::update() {
    // Update effects
    for (auto& effect : effects) {
        effect.update(deltaTime);
    }

    for (auto& wave : waves) {
        wave.update(deltaTime);
    }

    // Update flying planes
    for (auto& plane : planes) {
        plane.update(deltaTime);
    }

    if (currentState == GameState::BATTLE && !playerTurn && !attackingWithArsenal) {
        computerShoot();
        playerTurn = true;

        if (computerScore >= 20) {
            currentState = GameState::GAME_OVER;
        }
    }
}

void Game::computerShoot() {
    int x, y;

    if (!targetQueue.empty()) {
        auto target = targetQueue.front();
        targetQueue.erase(targetQueue.begin());
        x = target.first;
        y = target.second;
    }
    else {
        do {
            x = rand() % BOARD_SIZE;
            y = rand() % BOARD_SIZE;
        } while (playerBoard[y][x] == static_cast<int>(CellState::HIT) ||
            playerBoard[y][x] == static_cast<int>(CellState::MISS) ||
            playerBoard[y][x] == static_cast<int>(CellState::SUNK));
    }

    sf::Vector2f shotPos(BOARD_OFFSET_X + x * CELL_SIZE + CELL_SIZE / 2,
        BOARD_OFFSET_Y + y * CELL_SIZE + CELL_SIZE / 2);

    // Add enemy flying plane animation
    sf::Vector2f enemyPlaneStart(COMPUTER_BOARD_OFFSET_X - 50,
        BOARD_OFFSET_Y + BOARD_SIZE * CELL_SIZE / 2);
    addFlyingPlane(enemyPlaneStart, shotPos, false);

    if (playerBoard[y][x] == static_cast<int>(CellState::SHIP)) {
        playerBoard[y][x] = static_cast<int>(CellState::HIT);
        computerScore++;

        // Hit effect
        addEffect(shotPos, hitRed, 1.2f);
        addWave(shotPos, hitRed, 60.0f);

        // Check if ship is sunk
        checkAndMarkSunkShip(playerBoard, playerShips, x, y);

        if (targetQueue.empty()) {
            addAdjacentTargets(x, y);
        }
    }
    else {
        playerBoard[y][x] = static_cast<int>(CellState::MISS);

        // Miss effect
        addEffect(shotPos, missWhite, 0.8f);
        addWave(shotPos, lightBlue, 40.0f);
    }
}

void Game::checkAndMarkSunkShip(int board[BOARD_SIZE][BOARD_SIZE], std::vector<Ship>& ships, int hitX, int hitY) {
    for (auto& ship : ships) {
        if (!ship.placed) continue;

        // Check if this hit is on this ship
        bool isOnShip = false;
        int partIndex = -1;

        for (int i = 0; i < ship.size; i++) {
            int sx = ship.x + (ship.horizontal ? i : 0);
            int sy = ship.y + (ship.horizontal ? 0 : i);

            if (sx == hitX && sy == hitY) {
                isOnShip = true;
                partIndex = i;
                break;
            }
        }

        if (isOnShip) {
            ship.hitPart(partIndex);

            if (ship.sunk) {
                // Mark all ship cells as sunk
                for (int i = 0; i < ship.size; i++) {
                    int sx = ship.x + (ship.horizontal ? i : 0);
                    int sy = ship.y + (ship.horizontal ? 0 : i);
                    board[sy][sx] = static_cast<int>(CellState::SUNK);
                }

                // Add sunk ship effect
                sf::Vector2f shipCenter(
                    (ship.horizontal ? ship.x + ship.size / 2.0f : ship.x + 0.5f) * CELL_SIZE,
                    (ship.horizontal ? ship.y + 0.5f : ship.y + ship.size / 2.0f) * CELL_SIZE
                );

                if (board == playerBoard) {
                    shipCenter.x += BOARD_OFFSET_X;
                }
                else {
                    shipCenter.x += COMPUTER_BOARD_OFFSET_X;
                }
                shipCenter.y += BOARD_OFFSET_Y;

                addWave(shipCenter, sunkDarkRed, 80.0f);
            }
            break;
        }
    }
}

void Game::addAdjacentTargets(int x, int y) {
    int dx[] = { -1, 1, 0, 0 };
    int dy[] = { 0, 0, -1, 1 };

    for (int i = 0; i < 4; i++) {
        int nx = x + dx[i];
        int ny = y + dy[i];

        if (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE) {
            if (playerBoard[ny][nx] == static_cast<int>(CellState::SHIP) ||
                playerBoard[ny][nx] == static_cast<int>(CellState::EMPTY)) {
                targetQueue.push_back({ nx, ny });
            }
        }
    }
}

void Game::addEffect(sf::Vector2f pos, sf::Color color, float lifetime) {
    for (auto& effect : effects) {
        if (!effect.active) {
            effect.start(pos, color, lifetime);
            break;
        }
    }
}

void Game::addWave(sf::Vector2f pos, sf::Color color, float maxRadius) {
    for (auto& wave : waves) {
        if (!wave.active) {
            wave.start(pos, color, maxRadius);
            break;
        }
    }
}

void Game::updateHoveredCell() {
    if (currentState == GameState::SHIP_PLACEMENT) {
        int boardX = (mousePos.x - BOARD_OFFSET_X) / CELL_SIZE;
        int boardY = (mousePos.y - BOARD_OFFSET_Y) / CELL_SIZE;
        hoveredCell = sf::Vector2i(boardX, boardY);
    }
    else if (currentState == GameState::BATTLE) {
        int boardX = (mousePos.x - COMPUTER_BOARD_OFFSET_X) / CELL_SIZE;
        int boardY = (mousePos.y - BOARD_OFFSET_Y) / CELL_SIZE;
        hoveredCell = sf::Vector2i(boardX, boardY);
    }
}

bool Game::canPlaceShip(int board[BOARD_SIZE][BOARD_SIZE], int x, int y, int size, bool horizontal) {
    if (horizontal) {
        if (x + size > BOARD_SIZE) return false;
        for (int i = 0; i < size; i++) {
            if (board[y][x + i] != static_cast<int>(CellState::EMPTY)) return false;
        }
    }
    else {
        if (y + size > BOARD_SIZE) return false;
        for (int i = 0; i < size; i++) {
            if (board[y + i][x] != static_cast<int>(CellState::EMPTY)) return false;
        }
    }
    return true;
}

void Game::placeShip(int board[BOARD_SIZE][BOARD_SIZE], int x, int y, int size, bool horizontal) {
    if (horizontal) {
        for (int i = 0; i < size; i++) {
            board[y][x + i] = static_cast<int>(CellState::SHIP);
        }
    }
    else {
        for (int i = 0; i < size; i++) {
            board[y + i][x] = static_cast<int>(CellState::SHIP);
        }
    }

    markShipArea(board, x, y, size, horizontal);
}

void Game::markShipArea(int board[BOARD_SIZE][BOARD_SIZE], int x, int y, int size, bool horizontal) {
    int minX = std::max(0, x - 1);
    int maxX = std::min(BOARD_SIZE - 1, horizontal ? x + size : x + 1);
    int minY = std::max(0, y - 1);
    int maxY = std::min(BOARD_SIZE - 1, horizontal ? y + 1 : y + size);

    for (int i = minY; i <= maxY; i++) {
        for (int j = minX; j <= maxX; j++) {
            if (board[i][j] == static_cast<int>(CellState::EMPTY)) {
                board[i][j] = static_cast<int>(CellState::SHIP_AREA);
            }
        }
    }
}

void Game::placeComputerShips() {
    for (auto& ship : computerShips) {
        bool placed = false;
        int attempts = 0;

        while (!placed && attempts < 1000) {
            int x = rand() % BOARD_SIZE;
            int y = rand() % BOARD_SIZE;
            bool horizontal = rand() % 2 == 0;

            if (canPlaceShip(computerBoard, x, y, ship.size, horizontal)) {
                placeShip(computerBoard, x, y, ship.size, horizontal);
                ship.x = x;
                ship.y = y;
                ship.horizontal = horizontal;
                ship.placed = true;
                placed = true;
            }
            attempts++;
        }
    }
}

void Game::render() {
    window.clear(sf::Color(13, 26, 51));

    // Draw animated background
    drawAnimatedBackground();

    switch (currentState) {
    case GameState::MENU:
        renderMenu();
        break;
    case GameState::RULES:
        renderRules();
        break;
    case GameState::CREDITS:
        renderCredits();
        break;
    case GameState::SHIP_PLACEMENT:
        renderShipPlacement();
        break;
    case GameState::BATTLE:
        renderBattle();
        break;
    case GameState::GAME_OVER:
        renderGameOver();
        break;
    }

    if (selectingArsenal) {
        drawArsenalMenu();
    }

    if (attackingWithArsenal) {
        sf::Vector2f targetPos(COMPUTER_BOARD_OFFSET_X + arsenalTarget.x * CELL_SIZE + CELL_SIZE / 2,
            BOARD_OFFSET_Y + arsenalTarget.y * CELL_SIZE + CELL_SIZE / 2);
        draw3DAnimation(selectedArsenal, targetPos);
    }

    // Draw arsenal button
    if (currentState == GameState::BATTLE) {
        sf::RectangleShape arsenalBtn(sf::Vector2f(200, 40));
        arsenalBtn.setPosition(50, 50);
        arsenalBtn.setFillColor(sf::Color(100, 100, 200));
        window.draw(arsenalBtn);

        sf::Text arsenalText("Special Attack", font, 20);
        arsenalText.setPosition(60, 60);
        window.draw(arsenalText);
    }

    // Draw arsenal counts
    for (size_t i = 0; i < arsenals.size(); i++) {
        sf::Sprite icon(arsenals[i].icon);
        icon.setPosition(300 + i * 50, 50);
        icon.setScale(0.1f, 0.1f);
        window.draw(icon);

        sf::Text count(std::to_string(arsenals[i].count), font, 18);
        count.setPosition(320 + i * 50, 70);
        window.draw(count);
    }

    // Draw effects and animations
    drawEffects();
    drawPlanes();

    window.display();
}

void Game::drawAnimatedBackground() {
    sf::RectangleShape waterRect(sf::Vector2f(window.getSize().x, window.getSize().y));
    waterRect.setFillColor(sf::Color(13, 26, 51));
    window.draw(waterRect);

    sf::VertexArray waves(sf::LinesStrip);
    for (int x = 0; x <= window.getSize().x; x += 10) {
        float y = window.getSize().y * 0.8f +
            std::sin(x * 0.01f + waterAnimationTime * 2.0f) * 5.0f +
            std::cos(x * 0.015f + waterAnimationTime * 1.5f) * 3.0f;
        waves.append(sf::Vertex(sf::Vector2f(x, y), sf::Color(30, 80, 150, 100)));
    }
    window.draw(waves);

    for (int i = 0; i < 100; i++) {
        float starX = static_cast<float>(rand() % window.getSize().x);
        float starY = static_cast<float>(rand() % static_cast<int>(window.getSize().y * 0.6f));
        float size = 1.0f + static_cast<float>(rand() % 3);
        float alpha = 100 + rand() % 155;
        float twinkle = std::sin(waterAnimationTime * 5.0f + starX * 0.1f) * 0.5f + 0.5f;

        sf::CircleShape star(size);
        star.setPosition(starX, starY);
        star.setFillColor(sf::Color(255, 255, 255, static_cast<sf::Uint8>(alpha * twinkle)));
        window.draw(star);
    }
}

void Game::drawEffects() {
    for (auto& wave : waves) {
        wave.draw(window);
    }
    for (auto& effect : effects) {
        if (effect.active) {
            float radius = 10.0f * effect.scale;
            sf::CircleShape circle(radius);
            circle.setOrigin(radius, radius);
            circle.setPosition(effect.position);
            circle.setFillColor(effect.color);
            window.draw(circle);
        }
    }
}

void Game::drawPlanes() {
    for (auto& plane : planes) {
        plane.draw(window);
    }
}

void Game::renderMenu() {
    sf::Text title("BATTLESHIP", titleFont, 80);
    title.setFillColor(sf::Color(200, 200, 255));
    title.setStyle(sf::Text::Bold);
    title.setPosition(window.getSize().x / 2 - title.getGlobalBounds().width / 2, 50);
    window.draw(title);

    sf::Text option1("1. Start Game", font, 36);
    option1.setFillColor(sf::Color(200, 200, 255));
    option1.setPosition(window.getSize().x / 2 - option1.getGlobalBounds().width / 2, 200);
    window.draw(option1);

    sf::Text option2("2. Rules", font, 36);
    option2.setFillColor(sf::Color(200, 200, 255));
    option2.setPosition(window.getSize().x / 2 - option2.getGlobalBounds().width / 2, 260);
    window.draw(option2);

    sf::Text option3("3. Credits", font, 36);
    option3.setFillColor(sf::Color(200, 200, 255));
    option3.setPosition(window.getSize().x / 2 - option3.getGlobalBounds().width / 2, 320);
    window.draw(option3);

    sf::Text option4("4. Exit", font, 36);
    option4.setFillColor(sf::Color(200, 200, 255));
    option4.setPosition(window.getSize().x / 2 - option4.getGlobalBounds().width / 2, 380);
    window.draw(option4);

    sf::Text instructions("Use number keys to select options", font, 20);
    instructions.setFillColor(sf::Color(150, 150, 150));
    instructions.setPosition(window.getSize().x / 2 - instructions.getGlobalBounds().width / 2, 650);
    window.draw(instructions);
}

void Game::renderRules() {
    sf::Text title("RULES", titleFont, 60);
    title.setFillColor(sf::Color(200, 200, 255));
    title.setStyle(sf::Text::Bold);
    title.setPosition(window.getSize().x / 2 - title.getGlobalBounds().width / 2, 50);
    window.draw(title);

    std::vector<std::string> rules = {
        "1. Each player has a fleet of ships to place on their grid:",
        "   - 1 Battleship (4 cells)",
        "   - 2 Cruisers (3 cells each)",
        "   - 3 Destroyers (2 cells each)",
        "   - 4 Submarines (1 cell each)",
        "2. Players take turns firing shots at enemy ships",
        "3. Hit all parts of a ship to sink it",
        "4. First player to sink all enemy ships wins!",
        "",
        "During placement:",
        "- Press R to rotate ships",
        "- Click to place ships",
        "",
        "During battle:",
        "- Click on enemy grid to fire",
        "- Press D to toggle debug view (show enemy ships)"
    };

    for (size_t i = 0; i < rules.size(); i++) {
        sf::Text ruleText(rules[i], font, 20);
        ruleText.setFillColor(sf::Color(200, 200, 200));
        ruleText.setPosition(100, 150 + i * 30);
        window.draw(ruleText);
    }

    sf::Text back("Press ESC to return to menu", font, 24);
    back.setFillColor(sf::Color(150, 150, 150));
    back.setPosition(window.getSize().x / 2 - back.getGlobalBounds().width / 2, 650);
    window.draw(back);
}

void Game::renderCredits() {
    sf::Text title("CREDITS", titleFont, 60);
    title.setFillColor(sf::Color(200, 200, 255));
    title.setStyle(sf::Text::Bold);
    title.setPosition(window.getSize().x / 2 - title.getGlobalBounds().width / 2, 50);
    window.draw(title);

    std::vector<std::string> credits = {
        "Battleship Game",
        "Created with SFML",
        "",
        "Programming: Ruzimuhammad Alinazarov",
        "Graphics: Ruzimuhammad Alinazarov",
        "Sound Effects: Ruzimuhammad ALinazarov",
        "",
        "Special Thanks:",
        "- SFML Team",
        "- All Playtesters"
    };

    for (size_t i = 0; i < credits.size(); i++) {
        sf::Text creditText(credits[i], font, 24);
        creditText.setFillColor(sf::Color(200, 200, 200));
        creditText.setPosition(window.getSize().x / 2 - creditText.getGlobalBounds().width / 2,
            150 + i * 40);
        window.draw(creditText);
    }

    sf::Text back("Press ESC to return to menu", font, 24);
    back.setFillColor(sf::Color(150, 150, 150));
    back.setPosition(window.getSize().x / 2 - back.getGlobalBounds().width / 2, 650);
    window.draw(back);
}

void Game::renderShipPlacement() {
    sf::Text title("PLACE YOUR SHIPS", titleFont, 50);
    title.setFillColor(sf::Color(200, 200, 255));
    title.setStyle(sf::Text::Bold);
    title.setPosition(window.getSize().x / 2 - title.getGlobalBounds().width / 2, 30);
    window.draw(title);

    std::string instruction = "Place your " + getCurrentShipName();
    sf::Text instructionText(instruction, font, 24);
    instructionText.setFillColor(sf::Color(200, 200, 200));
    instructionText.setPosition(window.getSize().x / 2 - instructionText.getGlobalBounds().width / 2, 80);
    window.draw(instructionText);

    sf::Text helpText("Press R to rotate, Click to place", font, 20);
    helpText.setFillColor(sf::Color(150, 150, 150));
    helpText.setPosition(window.getSize().x / 2 - helpText.getGlobalBounds().width / 2, 110);
    window.draw(helpText);

    drawBoard(BOARD_OFFSET_X, BOARD_OFFSET_Y, playerBoard, true);

    if (currentShipIndex < playerShips.size()) {
        const Ship& currentShip = playerShips[currentShipIndex];
        int size = currentShip.size;
        bool horizontal = currentShip.horizontal;

        if (hoveredCell.x >= 0 && hoveredCell.x < BOARD_SIZE &&
            hoveredCell.y >= 0 && hoveredCell.y < BOARD_SIZE) {

            bool canPlace = canPlaceShip(playerBoard, hoveredCell.x, hoveredCell.y, size, horizontal);

            // Instead of drawing colored rectangles, draw the doodle ship
            if (canPlace) {
                // Draw the ship preview in the hovered position
                drawDoodleShip(hoveredCell.x, hoveredCell.y, size, horizontal, currentShip.type, false);
            }
            else {
                // If can't place, show red X over the area
                for (int i = 0; i < size; i++) {
                    int x = hoveredCell.x + (horizontal ? i : 0);
                    int y = hoveredCell.y + (horizontal ? 0 : i);

                    if (x < BOARD_SIZE && y < BOARD_SIZE) {
                        // Draw red X
                        sf::Vertex line1[] = {
                            sf::Vertex(sf::Vector2f(BOARD_OFFSET_X + x * CELL_SIZE + 5,
                                                  BOARD_OFFSET_Y + y * CELL_SIZE + 5),
                                          sf::Color::Red),
                            sf::Vertex(sf::Vector2f(BOARD_OFFSET_X + (x + 1) * CELL_SIZE - 5,
                                                  BOARD_OFFSET_Y + (y + 1) * CELL_SIZE - 5),
                                          sf::Color::Red)
                        };
                        sf::Vertex line2[] = {
                            sf::Vertex(sf::Vector2f(BOARD_OFFSET_X + (x + 1) * CELL_SIZE - 5,
                                                  BOARD_OFFSET_Y + y * CELL_SIZE + 5),
                                          sf::Color::Red),
                            sf::Vertex(sf::Vector2f(BOARD_OFFSET_X + x * CELL_SIZE + 5,
                                                  BOARD_OFFSET_Y + (y + 1) * CELL_SIZE - 5),
                                          sf::Color::Red)
                        };
                        window.draw(line1, 2, sf::Lines);
                        window.draw(line2, 2, sf::Lines);
                    }
                }
            }
        }
    }

    sf::Text shipsLeft("Ships remaining: " + std::to_string(playerShips.size() - currentShipIndex),
        font, 24);
    shipsLeft.setFillColor(sf::Color(200, 200, 200));
    shipsLeft.setPosition(BOARD_OFFSET_X, BOARD_OFFSET_Y + BOARD_SIZE * CELL_SIZE + 20);
    window.draw(shipsLeft);
}

void Game::renderBattle() {
    sf::Text title("BATTLE PHASE", titleFont, 50);
    title.setFillColor(sf::Color(200, 200, 255));
    title.setStyle(sf::Text::Bold);
    title.setPosition(window.getSize().x / 2 - title.getGlobalBounds().width / 2, 30);
    window.draw(title);

    sf::Text playerLabel("YOUR FLEET", font, 24);
    playerLabel.setFillColor(sf::Color(200, 200, 200));
    playerLabel.setPosition(BOARD_OFFSET_X, BOARD_OFFSET_Y - 60);
    window.draw(playerLabel);

    sf::Text computerLabel("ENEMY TERRITORY", font, 24);
    computerLabel.setFillColor(sf::Color(200, 200, 200));
    computerLabel.setPosition(COMPUTER_BOARD_OFFSET_X, BOARD_OFFSET_Y - 60);
    window.draw(computerLabel);

    drawBoard(BOARD_OFFSET_X, BOARD_OFFSET_Y, playerBoard, true);
    drawBoard(COMPUTER_BOARD_OFFSET_X, BOARD_OFFSET_Y, computerBoard, showingComputerShips);

    sf::Text turnText(playerTurn ? "YOUR TURN" : "ENEMY'S TURN", font, 30);
    turnText.setFillColor(playerTurn ? sf::Color(100, 255, 100) : sf::Color(255, 100, 100));
    turnText.setPosition(window.getSize().x / 2 - turnText.getGlobalBounds().width / 2,
        BOARD_OFFSET_Y + BOARD_SIZE * CELL_SIZE + 20);
    window.draw(turnText);

    sf::Text playerScoreText("Your hits: " + std::to_string(playerScore), font, 24);
    playerScoreText.setFillColor(sf::Color(200, 200, 200));
    playerScoreText.setPosition(BOARD_OFFSET_X, BOARD_OFFSET_Y + BOARD_SIZE * CELL_SIZE + 60);
    window.draw(playerScoreText);

    sf::Text computerScoreText("Enemy hits: " + std::to_string(computerScore), font, 24);
    computerScoreText.setFillColor(sf::Color(200, 200, 200));
    computerScoreText.setPosition(COMPUTER_BOARD_OFFSET_X, BOARD_OFFSET_Y + BOARD_SIZE * CELL_SIZE + 60);
    window.draw(computerScoreText);

    if (playerTurn && hoveredCell.x >= 0 && hoveredCell.x < BOARD_SIZE &&
        hoveredCell.y >= 0 && hoveredCell.y < BOARD_SIZE) {

        int cellState = computerBoard[hoveredCell.y][hoveredCell.x];
        bool canShoot = (cellState != static_cast<int>(CellState::HIT) &&
            cellState != static_cast<int>(CellState::MISS) &&
            cellState != static_cast<int>(CellState::SUNK));

        sf::RectangleShape cell(sf::Vector2f(CELL_SIZE - 2, CELL_SIZE - 2));
        cell.setPosition(COMPUTER_BOARD_OFFSET_X + hoveredCell.x * CELL_SIZE + 1,
            BOARD_OFFSET_Y + hoveredCell.y * CELL_SIZE + 1);
        cell.setFillColor(canShoot ? sf::Color(255, 255, 255, 50) : sf::Color(255, 100, 100, 50));
        window.draw(cell);
    }
}

void Game::renderGameOver() {
    bool playerWon = playerScore >= 20;

    sf::RectangleShape overlay(sf::Vector2f(window.getSize().x, window.getSize().y));
    overlay.setFillColor(sf::Color(0, 0, 0, 200));
    window.draw(overlay);

    sf::Text result(playerWon ? "VICTORY!" : "DEFEAT", titleFont, 80);
    result.setFillColor(playerWon ? sf::Color(100, 255, 100) : sf::Color(255, 100, 100));
    result.setStyle(sf::Text::Bold);
    result.setPosition(window.getSize().x / 2 - result.getGlobalBounds().width / 2, 150);
    window.draw(result);

    sf::Text scoreText("Final Score - You: " + std::to_string(playerScore) +
        "  Enemy: " + std::to_string(computerScore), font, 30);
    scoreText.setFillColor(sf::Color(200, 200, 200));
    scoreText.setPosition(window.getSize().x / 2 - scoreText.getGlobalBounds().width / 2, 250);
    window.draw(scoreText);

    sf::Text restart("Press R to play again", font, 30);
    restart.setFillColor(sf::Color(200, 200, 200));
    restart.setPosition(window.getSize().x / 2 - restart.getGlobalBounds().width / 2, 350);
    window.draw(restart);

    sf::Text menu("Press ESC to return to menu", font, 30);
    menu.setFillColor(sf::Color(200, 200, 200));
    menu.setPosition(window.getSize().x / 2 - menu.getGlobalBounds().width / 2, 400);
    window.draw(menu);
}

void Game::drawBoard(int offsetX, int offsetY, int board[BOARD_SIZE][BOARD_SIZE], bool showShips) {
    sf::RectangleShape gridBackground(sf::Vector2f(BOARD_SIZE * CELL_SIZE, BOARD_SIZE * CELL_SIZE));
    gridBackground.setPosition(offsetX, offsetY);
    gridBackground.setFillColor(sf::Color(30, 60, 120, 150));
    gridBackground.setOutlineThickness(2);
    gridBackground.setOutlineColor(sf::Color(50, 100, 200));
    window.draw(gridBackground);

    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            sf::RectangleShape cell(sf::Vector2f(CELL_SIZE - 2, CELL_SIZE - 2));
            cell.setPosition(offsetX + j * CELL_SIZE + 1, offsetY + i * CELL_SIZE + 1);

            switch (static_cast<CellState>(board[i][j])) {
            case CellState::EMPTY:
                cell.setFillColor(sf::Color(30, 60, 120, 100));
                window.draw(cell);
                break;

            case CellState::SHIP:
                if (showShips) {
                    // Skip drawing the rectangle - we'll draw the ship below
                }
                else {
                    cell.setFillColor(sf::Color(30, 60, 120, 100));
                    window.draw(cell);
                }
                break;

            case CellState::SHIP_AREA:
                cell.setFillColor(sf::Color(30, 60, 120, 100));
                window.draw(cell);
                break;

            case CellState::HIT:
                cell.setFillColor(hitRed);
                window.draw(cell);
                break;

            case CellState::MISS:
                cell.setFillColor(sf::Color(200, 200, 200, 150));
                window.draw(cell);
                break;

            case CellState::SUNK:
                cell.setFillColor(sunkDarkRed);
                window.draw(cell);
                break;
            }
        }
    }

    // Draw ships after the grid
    if (showShips) {
        const std::vector<Ship>& shipsToDraw = (board == playerBoard) ? playerShips : computerShips;
        for (const Ship& ship : shipsToDraw) {
            if (ship.placed) {
                drawDoodleShip(ship.x, ship.y, ship.size, ship.horizontal, ship.type, ship.sunk,
                    offsetX, offsetY);
            }
        }
    }

    sf::VertexArray gridLines(sf::Lines);
    for (int i = 0; i <= BOARD_SIZE; i++) {
        gridLines.append(sf::Vertex(sf::Vector2f(offsetX, offsetY + i * CELL_SIZE), sf::Color(200, 200, 200, 50)));
        gridLines.append(sf::Vertex(sf::Vector2f(offsetX + BOARD_SIZE * CELL_SIZE, offsetY + i * CELL_SIZE), sf::Color(200, 200, 200, 50)));
        gridLines.append(sf::Vertex(sf::Vector2f(offsetX + i * CELL_SIZE, offsetY), sf::Color(200, 200, 200, 50)));
        gridLines.append(sf::Vertex(sf::Vector2f(offsetX + i * CELL_SIZE, offsetY + BOARD_SIZE * CELL_SIZE), sf::Color(200, 200, 200, 50)));
    }
    window.draw(gridLines);

    for (int i = 0; i < BOARD_SIZE; i++) {
        sf::Text rowLabel(std::string(1, 'A' + i), font, 16);
        rowLabel.setFillColor(sf::Color(200, 200, 200));
        rowLabel.setPosition(offsetX - 25, offsetY + i * CELL_SIZE + CELL_SIZE / 2 - 8);
        window.draw(rowLabel);

        sf::Text colLabel(std::to_string(i + 1), font, 16);
        colLabel.setFillColor(sf::Color(200, 200, 200));
        colLabel.setPosition(offsetX + i * CELL_SIZE + CELL_SIZE / 2 - 4, offsetY - 25);
        window.draw(colLabel);
    }
}

std::string Game::getCurrentShipName() {
    if (currentShipIndex >= playerShips.size()) return "";
    switch (playerShips[currentShipIndex].type) {
    case ShipType::BATTLESHIP: return "Battleship (4 cells)";
    case ShipType::CRUISER: return "Cruiser (3 cells)";
    case ShipType::DESTROYER: return "Destroyer (2 cells)";
    case ShipType::SUBMARINE: return "Submarine (1 cell)";
    default: return "Ship";
    }
}

void Game::restartGame() {
    initializeBoards();
    initializeShips();
    placeComputerShips();
    currentShipIndex = 0;
    playerScore = 0;
    computerScore = 0;
    playerTurn = true;
    targetQueue.clear();
    currentState = GameState::SHIP_PLACEMENT;
}

void Game::drawDoodleShip(int x, int y, int size, bool horizontal, ShipType type, bool sunk,
    int offsetX, int offsetY) {
    if (texturesLoaded) {
        // Use textures if loaded
        sf::Sprite shipSprite;
        shipSprite.setTexture(shipTextures[size - 1]);

        // Handle orientation
        if (horizontal) {
            shipSprite.setRotation(0);
            shipSprite.setOrigin(0, 0);
        }
        else {
            shipSprite.setRotation(90);
            shipSprite.setOrigin(0, shipTextures[size - 1].getSize().y);
        }

        // Position on board
        float posX = offsetX + x * CELL_SIZE;
        float posY = offsetY + y * CELL_SIZE;
        shipSprite.setPosition(posX, posY);

        // Optional: gray out if sunk
        if (sunk) {
            shipSprite.setColor(sf::Color(180, 180, 180, 200));
        }
        else {
            shipSprite.setColor(sf::Color::White);
        }

        window.draw(shipSprite);
    }
    else {
        // Fallback to colored rectangles if textures not loaded
        sf::Color shipColor;
        switch (type) {
        case ShipType::BATTLESHIP: shipColor = sf::Color(120, 120, 120); break;
        case ShipType::CRUISER: shipColor = sf::Color(100, 100, 150); break;
        case ShipType::DESTROYER: shipColor = sf::Color(80, 120, 80); break;
        case ShipType::SUBMARINE: shipColor = sf::Color(60, 80, 120); break;
        default: shipColor = sf::Color(105, 105, 105);
        }

        if (sunk) {
            shipColor = sf::Color(100, 100, 100, 150);
        }

        for (int i = 0; i < size; i++) {
            sf::RectangleShape shipPart(sf::Vector2f(CELL_SIZE - 2, CELL_SIZE - 2));
            if (horizontal) {
                shipPart.setPosition(offsetX + (x + i) * CELL_SIZE + 1,
                    offsetY + y * CELL_SIZE + 1);
            }
            else {
                shipPart.setPosition(offsetX + x * CELL_SIZE + 1,
                    offsetY + (y + i) * CELL_SIZE + 1);
            }
            shipPart.setFillColor(shipColor);
            window.draw(shipPart);
        }
    }
}

void Game::initArsenals() {
    arsenals.push_back(Arsenal(ArsenalType::RADAR, 1));
    arsenals.push_back(Arsenal(ArsenalType::AIRPLANE, 3));
    arsenals.push_back(Arsenal(ArsenalType::HELICOPTER, 1));
}

void Game::drawArsenalMenu() {
    sf::RectangleShape menuBg(sf::Vector2f(300, 150));
    menuBg.setFillColor(sf::Color(50, 50, 100, 200));
    menuBg.setPosition(window.getSize().x / 2 - 150, window.getSize().y / 2 - 75);
    window.draw(menuBg);

    sf::Text title("Select Arsenal", font, 24);
    title.setPosition(window.getSize().x / 2 - title.getGlobalBounds().width / 2,
        window.getSize().y / 2 - 60);
    window.draw(title);

    // Draw arsenal options
    float buttonY = window.getSize().y / 2 - 20;
    for (size_t i = 0; i < arsenals.size(); i++) {
        if (arsenals[i].count > 0) {
            // Draw button
            sf::RectangleShape button(sf::Vector2f(200, 30));
            button.setPosition(window.getSize().x / 2 - 100, buttonY);
            button.setFillColor(sf::Color(100, 100, 200));
            window.draw(button);

            // Draw icon
            sf::Sprite icon(arsenals[i].icon);
            icon.setPosition(window.getSize().x / 2 - 90, buttonY + 5);
            icon.setScale(0.1f, 0.1f);
            window.draw(icon);

            // Draw text
            std::string name;
            if (arsenals[i].type == ArsenalType::RADAR) name = "Radar Scan";
            else if (arsenals[i].type == ArsenalType::AIRPLANE) name = "Air Strike";
            else name = "Helicopter Attack";

            sf::Text option(name + " (" + std::to_string(arsenals[i].count) + ")", font, 18);
            option.setPosition(window.getSize().x / 2 - 70, buttonY + 5);
            window.draw(option);

            buttonY += 40;
        }
    }
}

void Game::draw3DAnimation(ArsenalType type, sf::Vector2f target) {
    float animProgress = arsenalAnimationProgress;
    sf::Vector2f startPos(-100, target.y - 100);
    sf::Vector2f direction = target - startPos;
    float rotation = atan2(direction.y, direction.x) * 180.0f / 3.14159f;

    if (type == ArsenalType::AIRPLANE) {
        sf::Vector2f currentPos = startPos + (target - startPos) * animProgress;

        float scale = 1.0f + 0.5f * std::sin(animProgress * 3.14159f); // Simulate depth

        // Airplane body
        sf::ConvexShape body;
        body.setPointCount(5);
        body.setPoint(0, sf::Vector2f(0, 0));
        body.setPoint(1, sf::Vector2f(60, 10));
        body.setPoint(2, sf::Vector2f(65, 20));
        body.setPoint(3, sf::Vector2f(60, 30));
        body.setPoint(4, sf::Vector2f(0, 40));
        body.setFillColor(sf::Color(180, 180, 255));
        body.setOrigin(32, 20);
        body.setScale(scale, scale);
        body.setRotation(rotation);
        body.setPosition(currentPos);
        window.draw(body);

        // Wings
        sf::RectangleShape wing(sf::Vector2f(50, 5));
        wing.setFillColor(sf::Color(150, 150, 255));
        wing.setOrigin(25, 2.5f);
        wing.setRotation(rotation);
        wing.setScale(scale, scale);
        wing.setPosition(currentPos.x + 10 * std::cos(rotation * 3.14159f / 180.0f),
            currentPos.y + 10 * std::sin(rotation * 3.14159f / 180.0f));
        window.draw(wing);

        // Trail
        if (animProgress > 0.2f) {
            sf::Vertex trail[] = {
                sf::Vertex(currentPos, sf::Color(255, 255, 255, 150)),
                sf::Vertex(currentPos - sf::Vector2f(100, 0), sf::Color(255, 255, 255, 0))
            };
            window.draw(trail, 2, sf::Lines);
        }

        if (animProgress >= 1.0f) {
            sf::CircleShape explosion(50);
            explosion.setFillColor(sf::Color(255, 100, 0, 200));
            explosion.setOrigin(50, 50);
            explosion.setPosition(target);
            window.draw(explosion);
        }
    }
    else if (type == ArsenalType::RADAR) {
        // Draw radar effect
        sf::CircleShape radar(50 * (1.0f - animProgress));
        radar.setFillColor(sf::Color(0, 255, 0, 100));
        radar.setPosition(target.x - radar.getRadius(), target.y - radar.getRadius());
        window.draw(radar);

        // Draw radar lines
        for (int i = 0; i < 8; i++) {
            float angle = i * 45.0f + animProgress * 360.0f;
            sf::Vertex line[] = {
                sf::Vertex(target, sf::Color(0, 255, 0, 150)),
                sf::Vertex(target + sf::Vector2f(radar.getRadius() * cos(angle * 3.14159f / 180.0f),
                                               radar.getRadius() * sin(angle * 3.14159f / 180.0f)),
                          sf::Color(0, 255, 0, 50))
            };
            window.draw(line, 2, sf::Lines);
        }

        // Draw explosion when animation completes
        if (animProgress >= 1.0f) {
            sf::CircleShape explosion(30 * (1.0f - (animProgress - 1.0f)));
            explosion.setPosition(target.x - explosion.getRadius(),
                target.y - explosion.getRadius());
            explosion.setFillColor(sf::Color(255, 100, 0, 200));
            window.draw(explosion);
        }
    }
    else if (type == ArsenalType::HELICOPTER) {
        sf::Vector2f currentPos = target;
        float verticalOffset = 50 * (1.0f - animProgress);
        float scale = 1.0f + 0.3f * std::sin(animProgress * 3.14159f);

        // Body
        sf::RectangleShape body(sf::Vector2f(60, 20));
        body.setFillColor(sf::Color(150, 150, 150));
        body.setOrigin(30, 10);
        body.setScale(scale, scale);
        body.setPosition(currentPos.x, currentPos.y + verticalOffset);
        window.draw(body);

        // Tail
        sf::RectangleShape tail(sf::Vector2f(30, 5));
        tail.setFillColor(sf::Color(100, 100, 100));
        tail.setOrigin(0, 2.5f);
        tail.setScale(scale, scale);
        tail.setPosition(currentPos.x - 30 * scale, currentPos.y + verticalOffset);
        window.draw(tail);

        // Rotors
        static float rotorAngle = 0;
        rotorAngle += deltaTime * 720.0f;

        sf::RectangleShape rotor(sf::Vector2f(70, 3));
        rotor.setFillColor(sf::Color(200, 200, 200));
        rotor.setOrigin(35, 1.5f);
        rotor.setPosition(currentPos.x, currentPos.y + verticalOffset - 15);
        rotor.setRotation(rotorAngle);
        rotor.setScale(scale, scale);
        window.draw(rotor);

        sf::RectangleShape tailRotor(sf::Vector2f(20, 2));
        tailRotor.setFillColor(sf::Color(200, 200, 200));
        tailRotor.setOrigin(10, 1);
        tailRotor.setPosition(currentPos.x - 40 * scale, currentPos.y + verticalOffset);
        tailRotor.setRotation(rotorAngle * 2);
        tailRotor.setScale(scale, scale);
        window.draw(tailRotor);

        if (animProgress >= 1.0f) {
            sf::CircleShape explosion(50);
            explosion.setFillColor(sf::Color(255, 100, 0, 200));
            explosion.setOrigin(50, 50);
            explosion.setPosition(target);
            window.draw(explosion);
        }
    }

    arsenalAnimationProgress += deltaTime * 2.0f;
    if (arsenalAnimationProgress >= 1.0f) {
        attackingWithArsenal = false;
        arsenalAnimationProgress = 0;
        playerTurn = false;

        // Process the attack
        processArsenalAttack();
    }
}

void Game::processArsenalAttack() {
    int x = arsenalTarget.x;
    int y = arsenalTarget.y;

    switch (selectedArsenal) {
    case ArsenalType::RADAR:
        // Reveal a 3x3 area
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                if (x + i >= 0 && x + i < BOARD_SIZE && y + j >= 0 && y + j < BOARD_SIZE) {
                    if (computerBoard[y + j][x + i] == static_cast<int>(CellState::SHIP)) {
                        computerBoard[y + j][x + i] = static_cast<int>(CellState::HIT);
                    }
                }
            }
        }
        break;

    case ArsenalType::AIRPLANE:
        // Hit a straight line
        for (int i = 0; i < BOARD_SIZE; i++) {
            if (computerBoard[i][x] == static_cast<int>(CellState::SHIP)) {
                computerBoard[i][x] = static_cast<int>(CellState::HIT);
            }
        }
        break;

    case ArsenalType::HELICOPTER:
        // Hit a 3x3 area hard
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                if (x + i >= 0 && x + i < BOARD_SIZE && y + j >= 0 && y + j < BOARD_SIZE) {
                    if (computerBoard[y + j][x + i] == static_cast<int>(CellState::SHIP)) {
                        computerBoard[y + j][x + i] = static_cast<int>(CellState::HIT);
                    }
                    if (computerBoard[y + j][x + i] == static_cast<int>(CellState::EMPTY)) {
                        computerBoard[y + j][x + i] = static_cast<int>(CellState::MISS);
                    }
                }
            }
        }
        break;
    }

    // Decrement arsenal count
    for (auto& arsenal : arsenals) {
        if (arsenal.type == selectedArsenal) {
            arsenal.count--;
            break;
        }
    }
}

void Game::initializeBoards() {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            playerBoard[i][j] = static_cast<int>(CellState::EMPTY);
            computerBoard[i][j] = static_cast<int>(CellState::EMPTY);
        }
    }
}

void Game::initializeShips() {
    playerShips.clear();
    computerShips.clear();

    // Standard battleship fleet
    playerShips.push_back(Ship(4));  // Battleship
    playerShips.push_back(Ship(3));  // Cruiser
    playerShips.push_back(Ship(3));  // Cruiser
    playerShips.push_back(Ship(2));  // Destroyer
    playerShips.push_back(Ship(2));  // Destroyer
    playerShips.push_back(Ship(2));  // Destroyer
    playerShips.push_back(Ship(1));  // Submarine
    playerShips.push_back(Ship(1));  // Submarine
    playerShips.push_back(Ship(1));  // Submarine
    playerShips.push_back(Ship(1));  // Submarine

    for (const auto& ship : playerShips) {
        computerShips.push_back(Ship(ship.size));
    }
}

void Game::addFlyingPlane(sf::Vector2f start, sf::Vector2f end, bool isPlayerPlane) {
    for (auto& plane : planes) {
        if (!plane.active) {
            plane.start(start, end, isPlayerPlane);
            break;
        }
    }
}