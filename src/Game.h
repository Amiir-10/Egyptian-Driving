#ifndef GAME_H
#define GAME_H

#include <GL/glut.h>
#include <string>
#include "Car.h"
#include "Level.h"

enum GameState {
    MENU,
    LEVEL1,
    LEVEL2,
    GAME_OVER,
    WIN
};

class Game {
public:
    Game();
    ~Game();

    void init();
    void update();
    void render();
    void handleInput(unsigned char key, int x, int y);
    void handleSpecialInput(int key, int x, int y);
    void handleSpecialInputUp(int key, int x, int y);
    void handleMouse(int button, int state, int x, int y);
    void reshape(int w, int h);

private:
    GameState currentState;
    Car playerCar;
    Level* currentLevel;
    
    // Camera settings
    bool isThirdPerson;
    float cameraDistance;
    float cameraHeight;

    // Lighting
    float dayTime; // 0.0 to 1.0 representing time of day
    
    void setupLights();
    void setCamera();
    void drawText(float x, float y, std::string text);
    void drawMenu();
    void drawHUD();
};

#endif
