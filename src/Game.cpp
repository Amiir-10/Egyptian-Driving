#include "Game.h"
#include "Level1.h"
#include "Level2.h"
#include <cmath>
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Game::Game() {
    currentState = MENU;
    isThirdPerson = true;
    cameraDistance = 5.0f;
    cameraHeight = 2.5f;
    dayTime = 0.5f; // Noon
    currentLevel = nullptr;
}

Game::~Game() {
    if (currentLevel) delete currentLevel;
}

void Game::init() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);

    playerCar.reset(0, 0);
}

void Game::update() {
    if (currentState == LEVEL1) {
        if (!currentLevel) {
            currentLevel = new Level1();
            currentLevel->init();
            playerCar.reset(0, 0);
        }
        
        playerCar.update();
        currentLevel->update();
        
        if (currentLevel->checkCollisions(playerCar)) {
            playerCar.reset(0, 0); // Reset on collision
        }
        
        if (currentLevel->isFinished(playerCar)) {
            delete currentLevel;
            currentLevel = nullptr;
            currentState = LEVEL2;
        }

        // Day/Night cycle
        dayTime += 0.0005f;
        if (dayTime > 1.0f) dayTime = 0.0f;
    }
    else if (currentState == LEVEL2) {
        if (!currentLevel) {
            currentLevel = new Level2();
            currentLevel->init();
            playerCar.reset(0, 0); // Start at 0,0 for parking
        }

        playerCar.update();
        currentLevel->update();

        if (currentLevel->checkCollisions(playerCar)) {
            if (currentLevel->isFinished(playerCar)) {
                currentState = WIN; // Parked!
            } else {
                playerCar.reset(0, 0); // Hit obstacle
            }
        }
    }
    glutPostRedisplay();
}

void Game::setCamera() {
    float carX = playerCar.getX();
    float carZ = playerCar.getZ();
    float carRot = playerCar.getRotation() * M_PI / 180.0f;

    if (isThirdPerson) {
        // Camera behind and above
        float camX = carX - sin(carRot) * cameraDistance;
        float camZ = carZ - cos(carRot) * cameraDistance;
        gluLookAt(camX, cameraHeight, camZ, carX, 0.5f, carZ, 0.0f, 1.0f, 0.0f);
    } else {
        // First person (driver's eye)
        float eyeX = carX + sin(carRot) * 0.5f;
        float eyeZ = carZ + cos(carRot) * 0.5f;
        float lookX = carX + sin(carRot) * 10.0f;
        float lookZ = carZ + cos(carRot) * 10.0f;
        gluLookAt(eyeX, 0.8f, eyeZ, lookX, 0.8f, lookZ, 0.0f, 1.0f, 0.0f);
    }
}

void Game::setupLights() {
    // Simple day/night cycle logic
    float ambient = 0.2f;
    float diffuse = 0.8f;
    
    if (dayTime > 0.75f || dayTime < 0.25f) { // Night
        ambient = 0.1f;
        diffuse = 0.2f;
        glClearColor(0.0f, 0.0f, 0.1f, 1.0f); // Dark blue sky
    } else { // Day
        glClearColor(0.5f, 0.8f, 1.0f, 1.0f); // Light blue sky
    }

    GLfloat lightPos[] = { 0.0f, 10.0f, 0.0f, 1.0f };
    // Rotate sun based on time
    lightPos[0] = sin(dayTime * 2 * M_PI) * 100.0f;
    lightPos[1] = cos(dayTime * 2 * M_PI) * 100.0f;

    GLfloat lightAmb[] = { ambient, ambient, ambient, 1.0f };
    GLfloat lightDif[] = { diffuse, diffuse, diffuse, 1.0f };
    
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmb);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDif);
}

void Game::render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    if (currentState == MENU) {
        drawMenu();
    } else {
        setCamera();
        setupLights();

        // Draw Ground
        glColor3f(0.9f, 0.8f, 0.6f); // Sand
        // Draw Ground (Tessellated for lighting)
        glColor3f(0.9f, 0.8f, 0.6f); // Sand
        glNormal3f(0, 1, 0);
        
        float groundSize = 200.0f;
        float tileSize = 2.0f; // Smaller tiles for better lighting
        
        glBegin(GL_QUADS);
        for (float x = -groundSize/2; x < groundSize/2; x += tileSize) {
            for (float z = -groundSize/2; z < groundSize/2; z += tileSize) {
                glVertex3f(x, 0, z);
                glVertex3f(x + tileSize, 0, z);
                glVertex3f(x + tileSize, 0, z + tileSize);
                glVertex3f(x, 0, z + tileSize);
            }
        }
        glEnd();

        if (currentLevel) {
            bool isNight = (dayTime > 0.75f || dayTime < 0.25f);
            currentLevel->render(playerCar, isNight);
        }
        playerCar.draw();
    }

    drawHUD();
    glutSwapBuffers();
}

void Game::handleInput(unsigned char key, int x, int y) {
    if (key == 27) exit(0); // ESC
    
    if (currentState == MENU) {
        if (key == 13) currentState = LEVEL1; // Enter
    } else {
        if (key == 'l' || key == 'L') playerCar.toggleLights();
    }
}

void Game::handleSpecialInput(int key, int x, int y) {
    if (currentState == LEVEL1 || currentState == LEVEL2) {
        switch(key) {
            case GLUT_KEY_UP: playerCar.accelerate(true); break;
            case GLUT_KEY_DOWN: playerCar.brake(true); break;
            case GLUT_KEY_LEFT: playerCar.turnLeft(true); break;
            case GLUT_KEY_RIGHT: playerCar.turnRight(true); break;
        }
    }
}

void Game::handleSpecialInputUp(int key, int x, int y) {
    if (currentState == LEVEL1 || currentState == LEVEL2) {
        switch(key) {
            case GLUT_KEY_UP: playerCar.accelerate(false); break;
            case GLUT_KEY_DOWN: playerCar.brake(false); break;
            case GLUT_KEY_LEFT: playerCar.turnLeft(false); break;
            case GLUT_KEY_RIGHT: playerCar.turnRight(false); break;
        }
    }
}

void Game::handleMouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        isThirdPerson = !isThirdPerson;
    }
}

void Game::drawText(float x, float y, std::string text) {
    glDisable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 800, 0, 600);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(x, y);
    for (char c : text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_LIGHTING);
}

void Game::drawMenu() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    drawText(300, 400, "EGYPTIAN DRIVING GAME");
    drawText(280, 300, "Press ENTER to Start Level 1");
}

void Game::drawHUD() {
    if (currentState != MENU) {
        std::string mode = isThirdPerson ? "3rd Person" : "1st Person";
        drawText(10, 580, "Camera: " + mode + " (Click to toggle)");
        drawText(10, 550, "Controls: Arrows to Move, L for Lights");
        
        if (currentState == LEVEL1) {
            int dist = (int)playerCar.getZ();
            if (dist < 0) dist = 0;
            std::string distStr = "Distance: " + std::to_string(dist) + " / 1000 m";
            drawText(10, 520, distStr);
        } else if (currentState == LEVEL2) {
            drawText(10, 520, "Park in the white spot!");
        }
    }
}

void Game::reshape(int w, int h) {
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (float)w / h, 0.1f, 1000.0f);
    glMatrixMode(GL_MODELVIEW);
}
