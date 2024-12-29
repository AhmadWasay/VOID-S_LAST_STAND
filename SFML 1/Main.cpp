﻿#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <cmath>  
#include <cstdlib>  
#include <ctime>  
#include "Player.h"
#include "Bullet.h"
#include "Enemy.h"
#include "HealthBar.h"

using namespace std;

const float MAX_PLAYER_HEALTH = 100.0f;
const float ENEMY_HEALTH = 2.0f;

enum class GameState {
    MainMenu,
    Playing,
    Paused,
    GameOver
};
GameState gameState = GameState::MainMenu;


sf::Vector2f WORLD_SIZE(1500.0f, 1500.0f);

static const float VIEW_HEIGHT = 512;
float SPAWN_INTERVAL = 2.0f;
float BULLET_INTERVAL = 0.2f;

static void ResizeView(const sf::RenderWindow& window, sf::View& view)
{
    float aspectRatio = static_cast<float>(window.getSize().x) / static_cast<float>(window.getSize().y);
    view.setSize(VIEW_HEIGHT * aspectRatio, VIEW_HEIGHT);
}

void highlightText(sf::Text& text, sf::Vector2f worldMousePos, int defaultSize, int hoverSize) {
    bool isMouseOver = text.getGlobalBounds().contains(worldMousePos);

    if (isMouseOver) {
        text.setCharacterSize(hoverSize);
        text.setFillColor(sf::Color::Transparent);
        text.setOutlineColor(sf::Color::White);
        text.setOutlineThickness(1.0f);
    }
    else {
        text.setCharacterSize(defaultSize);
        text.setFillColor(sf::Color::White);
        text.setOutlineThickness(0.0f);
    }
}


int main() {
    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;

    sf::RenderWindow window(sf::VideoMode(1000, 700), "SFML Tutorial", sf::Style::Close | sf::Style::Fullscreen);
    sf::View view(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(VIEW_HEIGHT, VIEW_HEIGHT));

    sf::Texture cursorTexture;
    if (!cursorTexture.loadFromFile("textures/aim.png")) {
        cerr << "Error loading cursor texture!" << endl;
        return -1;
    }

    sf::Sprite cursorSprite(cursorTexture);
    cursorSprite.setOrigin(cursorTexture.getSize().x / 2.0f, cursorTexture.getSize().y / 2.0f);
    cursorSprite.setScale(0.5f, 0.5f);

    sf::Texture bulletTexture;
    if (!bulletTexture.loadFromFile("textures/11.png"))
    {
        cerr << "Error loading cursor texture!" << endl;
        return -1;
    }

    sf::Texture blastTexture;
    if (!blastTexture.loadFromFile("textures/smoke.png")) {
        cerr << "Error loading blast Texture!" << endl;
        return -1;
    }
    sf::Sprite blastSprite(blastTexture);
    Animation blastAnimation(&blastTexture, sf::Vector2u(11, 15), 0.2f);
    sf::RectangleShape blastBody;
    sf::Vector2f blastSize(10.0f, 10.0f);
    blastBody.setSize(blastSize);
    blastBody.setOrigin(blastSize / 2.0f);  // Set origin to the center
    blastBody.setFillColor(sf::Color(50, 250, 50));
    blastBody.setTexture(&blastTexture);



    sf::Texture backgroundTexture;
    if (!backgroundTexture.loadFromFile("textures/purple.png")) {
        cerr << "Error loading background texture!" << endl;
        return -1;
    }
    sf::Sprite backgroundSprite(backgroundTexture);
    backgroundSprite.setOrigin(backgroundSprite.getLocalBounds().width / 2, backgroundSprite.getLocalBounds().height / 2);

    backgroundSprite.setScale(
        WORLD_SIZE.x / backgroundTexture.getSize().y * 2,
        WORLD_SIZE.y / backgroundTexture.getSize().y * 2
    );
    backgroundSprite.setPosition(window.getSize().x / 2,
        window.getSize().y / 2);
    backgroundSprite.setColor(sf::Color(255, 155, 155, 160));


    srand(static_cast<unsigned int>(time(0)));

    sf::Text timerText;
    sf::Text scoreText;
    sf::Font font;
    if (!font.loadFromFile("Fonts/V.ttf")) {
        cerr << "Error loading font!" << endl;
        return -1;
    }

    timerText.setFont(font);
    timerText.setCharacterSize(30);
    timerText.setFillColor(sf::Color::White);

    scoreText.setFont(font);
    scoreText.setCharacterSize(20);
    scoreText.setFillColor(sf::Color::White);

    sf::Text healthLabel;
    healthLabel.setFont(font);
    healthLabel.setCharacterSize(20);
    healthLabel.setFillColor(sf::Color::White);
    healthLabel.setString("Health");

    sf::Text scoreLabel;
    scoreLabel.setFont(font);
    scoreLabel.setCharacterSize(20);
    scoreLabel.setFillColor(sf::Color::White);
    scoreLabel.setString("Score");
    scoreLabel.setOrigin(scoreLabel.getLocalBounds().width / 2, scoreLabel.getLocalBounds().height / 2);



    sf::Text playText;
    playText.setFont(font);
    playText.setCharacterSize(40);
    playText.setFillColor(sf::Color::White);
    playText.setString("Play");
    playText.setOrigin(playText.getLocalBounds().width / 2, playText.getLocalBounds().height / 2);


    sf::Text GameOverText;
    GameOverText.setFont(font);
    GameOverText.setCharacterSize(60);
    GameOverText.setFillColor(sf::Color::White);
    GameOverText.setString("GameOver");
    GameOverText.setOrigin(GameOverText.getLocalBounds().width / 2, GameOverText.getLocalBounds().height / 2);

    bool GameOver = false;



    sf::Text titleText;
    titleText.setFont(font);
    titleText.setCharacterSize(60);
    titleText.setFillColor(sf::Color::White);
    titleText.setString("VOID'S LAST STAND");
    titleText.setOrigin(titleText.getLocalBounds().width / 2, titleText.getLocalBounds().height / 2);

    sf::Text exitText;
    exitText.setFont(font);
    exitText.setCharacterSize(40);
    exitText.setFillColor(sf::Color::White);
    exitText.setString("Exit");
    exitText.setPosition(window.getSize().x / 2 - 50, window.getSize().y / 2 + 100);

    sf::Text pauseText;
    pauseText.setFont(font);
    pauseText.setCharacterSize(60);
    pauseText.setFillColor(sf::Color::White);
    pauseText.setString("PAUSED");
    pauseText.setOrigin(pauseText.getLocalBounds().width / 2, pauseText.getLocalBounds().height / 2);

    sf::Text resumeText;
    resumeText.setFont(font);
    resumeText.setCharacterSize(40);
    resumeText.setFillColor(sf::Color::White);
    resumeText.setString("Resume");
    resumeText.setOrigin(resumeText.getLocalBounds().width / 2, resumeText.getLocalBounds().height / 2);

    sf::Text menuText;
    menuText.setFont(font);
    menuText.setCharacterSize(40);
    menuText.setFillColor(sf::Color::White);
    menuText.setString("Main Menu");
    menuText.setOrigin(menuText.getLocalBounds().width / 2, menuText.getLocalBounds().height / 2);


    int score = 0;


    // First call for consisten
    ResizeView(window, view);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Bullet Sound
    sf::SoundBuffer bulletSoundBuffer;
    if (!bulletSoundBuffer.loadFromFile("Sounds/laser.wav"))
    {
        cerr << "Error loading bullet sound!" << endl;
        return -1;
    }

    sf::Sound bulletSound;
    bulletSound.setBuffer(bulletSoundBuffer);

    sf::SoundBuffer enemySoundBuffer;
    if (!enemySoundBuffer.loadFromFile("Sounds/explode.mp3"))
    {
        cerr << "Error loading enemy explosion sound!" << endl;
        return -1;
    }

    sf::Sound enemySound;
    enemySound.setBuffer(enemySoundBuffer);

    // Hover Sound
    sf::SoundBuffer transitionSoundBuffer;
    if (!transitionSoundBuffer.loadFromFile("Sounds/Transition_3.wav"))
    {
        cerr << "Error loading hover sound!" << endl;
        return -1;
    }
    sf::Sound transitionSound;
    transitionSound.setBuffer(transitionSoundBuffer);


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Player
    sf::Texture playerTexture;
    if (!playerTexture.loadFromFile("textures/p3.png")) {
        cerr << "Error loading player texture!" << endl;
        return -1;
    }

    sf::Texture enemyTexture;
    if (!enemyTexture.loadFromFile("textures/ship.png")) {
        cerr << "Error loading enemy texture!" << endl;
        return -1;
    }

    sf::Texture enemyTexture2;
    if (!enemyTexture2.loadFromFile("textures/neye.png")) {
        cerr << "Error loading enemy2 texture!" << endl;
        return -1;
    }
    bool e = false;

    Player player(&playerTexture, sf::Vector2u(1, 1), 2.0f, MAX_PLAYER_HEALTH);

    float deltaTime = 0.0f;
    sf::Clock clock;

    // Boundry Wall
    sf::FloatRect worldBounds(0, 0, WORLD_SIZE.x, WORLD_SIZE.y);
    sf::RectangleShape boundary(sf::Vector2f(worldBounds.width - 2, worldBounds.height));
    boundary.setPosition(worldBounds.left - 5, worldBounds.top - 5);
    boundary.setOutlineColor(sf::Color(100, 200, 200));
    boundary.setOutlineThickness(2);
    boundary.setFillColor(sf::Color::Transparent);



    window.setFramerateLimit(60);  // Limits FPS to 60

    vector<Bullet*> bullets;
    vector<Enemy*> enemies;

    float spawnTimer = 0.0f;
    float bulletTimer = 0.0f;

    //////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Game loop
    while (window.isOpen()) {

        window.setMouseCursorVisible(true);

        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        sf::Vector2f worldMousePos = window.mapPixelToCoords(mousePos, view);

        deltaTime = clock.restart().asSeconds();
        sf::Event evnt;

        while (window.pollEvent(evnt)) {
            switch (evnt.type) {
            case sf::Event::Closed:
                window.close();
                break;
            case sf::Event::KeyPressed:
                if (evnt.key.code == sf::Keyboard::Escape) {
                    if (gameState == GameState::Playing) {
                        gameState = GameState::Paused;
                    }
                    else if (gameState == GameState::Paused) {
                        gameState = GameState::Playing;
                    }
                }
                break;
            case sf::Event::Resized:
                ResizeView(window, view);
                cout << window.getSize().x << ":" << window.getSize().y << endl;
                break;
            case sf::Event::TextEntered:
                if (evnt.text.unicode < 128)
                    printf(" %c", evnt.text.unicode);
                break;
            }
        }

        if (gameState == GameState::MainMenu) {
            // Highlight the texts


            highlightText(playText, worldMousePos, 40, 45);
            highlightText(exitText, worldMousePos, 40, 45);

            // Center the view for menu
            view.setCenter(sf::Vector2f(0.0f, 0.0f));
            window.setView(view);

            // Center the title text
            sf::FloatRect titleBounds = titleText.getLocalBounds();
            titleText.setOrigin(titleBounds.width / 2.0f, titleBounds.height / 2.0f);
            titleText.setPosition(view.getCenter().x, view.getCenter().y - 100.0f);

            // Center the play text
            sf::FloatRect playBounds = playText.getLocalBounds();
            playText.setOrigin(playBounds.width / 2.0f, playBounds.height / 2.0f);
            playText.setPosition(view.getCenter().x, view.getCenter().y);

            // Center the exit text
            sf::FloatRect exitBounds = exitText.getLocalBounds();
            exitText.setOrigin(exitBounds.width / 2.0f, exitBounds.height / 2.0f);
            exitText.setPosition(view.getCenter().x, view.getCenter().y + 50.0f);

            // Handle mouse click
            if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                if (playText.getGlobalBounds().contains(worldMousePos)) {
                    transitionSound.play();
                    gameState = GameState::Playing; // Start the game
                }
                else if (exitText.getGlobalBounds().contains(worldMousePos)) {
                    transitionSound.play();
                    window.close();
                }
            }

            // Draw the menu
            window.clear();
            window.draw(backgroundSprite);

            window.draw(titleText);
            window.draw(playText);
            window.draw(exitText);
            window.display();
        }




        else if (gameState == GameState::Playing) {
            // Game logic and rendering
            window.clear();

            window.setMouseCursorVisible(false);








            static float elapsedTime = 0;
            elapsedTime += deltaTime;

            if (SPAWN_INTERVAL > 1.0f)
                SPAWN_INTERVAL -= deltaTime * deltaTime;
            else
                SPAWN_INTERVAL = 1.0f;

            int minutes = static_cast<int>(elapsedTime) / 60;
            int seconds = static_cast<int>(elapsedTime) % 60;

            string timeString = (minutes < 10 ? "0" : "") + to_string(minutes) + ":"
                + (seconds < 10 ? "0" : "") + to_string(seconds);

            timerText.setString(timeString);


            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            sf::Vector2f worldMousePos = window.mapPixelToCoords(mousePos, view);

            sf::Vector2i worldMousePosInt(static_cast<int>(worldMousePos.x), static_cast<int>(worldMousePos.y));

            player.Update(deltaTime, worldMousePosInt);
            sf::Vector2f playerPos = player.GetPosition();


            ////////////////////////////////////////////////////////////////////////////////////////////
            ////////////////////////////////////////////////////////////////////////////////////////////

            // Spawn new enemies after the interval
            spawnTimer += deltaTime;
            if (spawnTimer >= SPAWN_INTERVAL)
            {
                spawnTimer = 0.0f;

                float spawnX = (rand() % 2 == 0) ? -50.0f : window.getSize().x + 50.0f;
                float spawnY = (rand() % 2 == 0) ? -50.0f : window.getSize().y + 50.0f;

                spawnX += rand() % 100 - 50; // -50 to 49 random 
                spawnY += rand() % 100 - 50;

                if (e)
                {
                    enemies.push_back(new Enemy(&enemyTexture, sf::Vector2u(5, 2), 0.02, sf::Vector2f(30.0f, 40.0f), sf::Vector2f(spawnX, spawnY), ENEMY_HEALTH));
                    e = false;
                }
                else
                {
                    enemies.push_back(new Enemy(&enemyTexture2, sf::Vector2u(10, 1), 0.02, sf::Vector2f(40.0f, 40.0f), sf::Vector2f(spawnX, spawnY), ENEMY_HEALTH));
                    e = true;
                }

            }

            bulletTimer += deltaTime;
            if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && bulletTimer >= BULLET_INTERVAL)
            {
                bullets.push_back(new Bullet(&bulletTexture, player.GetPosition(), worldMousePosInt, 800.0, 3.0f, 20.0));
                bulletSound.play();
                bulletTimer = 0;
            }
            //bullets.push_back(new Bullet(nullptr,  player.GetPosition(), worldMousePosInt, 0.2, 0.2f));

            for (auto& enemy : enemies) {
                if (enemy->GetCollider().CheckCollision(player.GetCollider(), 0.0f))
                {
                    player.setDead(true);
                }
            }

            view.setCenter(player.GetPosition());


            //////////////////////////////////////////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////////////////////////////////////////

            // UI / UX
            timerText.setPosition(player.GetPosition().x, player.GetPosition().y - view.getSize().y / 2.2);
            timerText.setOrigin(timerText.getLocalBounds().width / 2, timerText.getLocalBounds().height / 2);  // Set origin to the center of the text

            HealthBar healthBar(MAX_PLAYER_HEALTH, sf::Vector2f(player.GetPosition().x - view.getSize().x / 2.2, player.GetPosition().y - view.getSize().y / 2.3), sf::Vector2f(150.0f, 5.0f));
            scoreText.setPosition(player.GetPosition().x + view.getSize().x / 3, player.GetPosition().y - view.getSize().y / 2.3);
            healthLabel.setPosition(player.GetPosition().x - view.getSize().x / 2.6, player.GetPosition().y - view.getSize().y / 2.1f);
            scoreLabel.setPosition(player.GetPosition().x + view.getSize().x / 2.85, player.GetPosition().y - view.getSize().y / 2.1f);

            timerText.setOrigin(timerText.getLocalBounds().width / 2, timerText.getLocalBounds().height / 2);
            scoreText.setOrigin(scoreText.getLocalBounds().width / 2, scoreText.getLocalBounds().height / 2);
            healthLabel.setOrigin(healthLabel.getLocalBounds().width / 2, healthLabel.getLocalBounds().height / 2);

            healthBar.Update(player.GetHealth());

            playText.setPosition(view.getCenter().x, view.getCenter().y);
            titleText.setPosition(view.getCenter().x, view.getCenter().y - 40);
            pauseText.setPosition(view.getCenter().x, view.getCenter().y / 2 - 40);
            resumeText.setPosition(view.getCenter().x, view.getCenter().y - 20);
            menuText.setPosition(view.getCenter().x, view.getCenter().y + 40);
            ////////////////////////////////////////////////////////////////////////////////////////////////////////////
            ////////////////////////////////////////////////////////////////////////////////////////////////////////////

            // Player clamping (keep player inside world bounds)
            sf::Vector2f playerSize = player.GetSize();

            if (playerPos.x < worldBounds.left)
                playerPos.x = worldBounds.left;

            if (playerPos.x + playerSize.x > worldBounds.width)
                playerPos.x = worldBounds.width - playerSize.x;

            if (playerPos.y < worldBounds.top)
                playerPos.y = worldBounds.top;

            if (playerPos.y + playerSize.y > worldBounds.height)
                playerPos.y = worldBounds.height - playerSize.y;

            player.setPosition(playerPos.x, playerPos.y);


            /////////////////////////////////////////////////////////////////////////////////////////////////
            /////////////////////////////////////////////////////////////////////////////////////////////////

            // Check Enemies-Enemies Collisions
            for (size_t i = 0; i < enemies.size(); ++i)
                for (size_t j = i + 1; j < enemies.size(); ++j)
                    enemies[i]->GetCollider().CheckCollision(enemies[j]->GetCollider(), 0.0f);


            cursorSprite.setPosition(worldMousePos);

            window.clear();

            window.draw(backgroundSprite);
            window.draw(boundary);


            // Draw the player

            for (auto bullet = bullets.begin(); bullet != bullets.end();) {
                bool hit = false;

                // Check for bullet-enemy collisions
                for (auto& enemy : enemies) {
                    if ((*bullet)->GetCollider().CheckCollision(enemy->GetCollider(), 2.5f)) {
                        enemy->setDead(true);

                        (*bullet)->setDead(true);
                        score++;

                        // Delete the bullet
                        if ((*bullet)->isDead()) {

                            //blastBody.setPosition(enemy->getPosition());


                            //// Update the animation to get the first frame (row 0, for example)
                            //blastAnimation.update(0, deltaTime);
                            //// Set the texture rectangle of the sprite
                            //blastBody.setTextureRect(blastAnimation.uvRect);


                            

                            delete* bullet;  // Free memory
                        }
                        bullet = bullets.erase(bullet);  // Erase the bullet from the vector
                        hit = true;
                        break;
                    }
                }

                // Draw the cursor sprite for visual feedback

                // Check and delete dead enemies
                for (auto enemy = enemies.begin(); enemy != enemies.end();) {
                    if ((*enemy)->isDead()) {
                        blastSprite.setPosition((*enemy)->getPosition());
                        delete* enemy;  // Delete the enemy
                        
                        enemySound.play();  // Play sound when an enemy dies
                        enemy = enemies.erase(enemy);  // Erase the enemy from the vector
                    }
                    else {
                        ++enemy;  // Move to the next enemy if not dead
                    }
                }

                // If the bullet wasn't hit by any enemy, just continue to the next bullet
                if (!hit) {
                    (*bullet)->update(deltaTime);

                    if ((*bullet)->isDead()) {
                        delete* bullet;
                        bullet = bullets.erase(bullet);
                    }
                    else {
                        (*bullet)->Draw(window);
                        ++bullet;
                    }
                }
            }


            player.Draw(window);
            string scoreString = to_string(score);
            scoreText.setString(scoreString);




            for (auto& enemy : enemies)
                enemy->Update(playerPos, deltaTime);

            


            for (auto& enemy : enemies)
                enemy->Draw(window);

            //window.draw(blastSprite);

            window.draw(timerText);
            window.draw(scoreText);
            window.draw(healthLabel);
            window.draw(scoreLabel);
            healthBar.Draw(window);
            window.draw(cursorSprite);

            window.setView(view);
            window.display();

            if (player.isDead())
            {
                gameState = GameState::GameOver;
            }




        }
        if (gameState == GameState::Paused) {
            // Highlight the texts for pause menu options
            highlightText(resumeText, worldMousePos, 40, 45);
            highlightText(menuText, worldMousePos, 40, 45);

            // Center the view for the pause menu
            view.setCenter(sf::Vector2f(0.0f, 0.0f));
            window.setView(view);

            // Center the pause text
            sf::FloatRect pauseBounds = pauseText.getLocalBounds();
            pauseText.setOrigin(pauseBounds.width / 2.0f, pauseBounds.height / 2.0f);
            pauseText.setPosition(view.getCenter().x, view.getCenter().y - 100.0f);

            // Center the resume text
            sf::FloatRect resumeBounds = resumeText.getLocalBounds();
            resumeText.setOrigin(resumeBounds.width / 2.0f, resumeBounds.height / 2.0f);
            resumeText.setPosition(view.getCenter().x, view.getCenter().y);

            // Center the menu text
            sf::FloatRect menuBounds = menuText.getLocalBounds();
            menuText.setOrigin(menuBounds.width / 2.0f, menuBounds.height / 2.0f);
            menuText.setPosition(view.getCenter().x, view.getCenter().y + 50.0f);

            // Handle mouse click
            if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                if (resumeText.getGlobalBounds().contains(worldMousePos)) {
                    transitionSound.play();
                    gameState = GameState::Playing; // Resume the game
                }
                else if (menuText.getGlobalBounds().contains(worldMousePos)) {
                    transitionSound.play();
                    gameState = GameState::MainMenu; // Return to the main menu
                }
            }

            // Draw the pause menu
            window.clear();
            window.draw(backgroundSprite);

            
            window.draw(pauseText);
            window.draw(resumeText);
            window.draw(menuText);
            window.display();
        }


        else if (gameState == GameState::GameOver) {
            // Handle Game Over state

            //highlightText(exitText, worldMousePos, 40, 45);

            // Center the view for menu
            view.setCenter(sf::Vector2f(0.0f, 0.0f));
            window.setView(view);

            // Center the title text
            sf::FloatRect gameOverBounds = GameOverText.getLocalBounds();
            GameOverText.setOrigin(gameOverBounds.width / 2.0f, gameOverBounds.height / 2.0f);
            GameOverText.setPosition(view.getCenter().x, view.getCenter().y - 100.0f);

            sf::FloatRect exitBounds = exitText.getLocalBounds();
            exitText.setOrigin(exitBounds.width / 2.0f, exitBounds.height / 2.0f);
            exitText.setPosition(view.getCenter().x, view.getCenter().y + 50.0f);

            // Handle mouse click
            if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                if (exitText.getGlobalBounds().contains(worldMousePos)) {
                    transitionSound.play();
                    window.close();
                }
            }

            window.clear();
            window.draw(backgroundSprite);



            window.draw(GameOverText);

            // Optionally, add logic to restart the game or exit
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Enter)) {
                // Restart the game
                gameState = GameState::MainMenu; // or reset game variables as needed
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
                window.close(); // Exit the game
            }

            window.display();
        }
    }

    return 0;
}
