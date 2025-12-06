#include <GL/glut.h>
#include "Game.h"
#include <iostream>
#include <fstream>

Game game;

void display() {
    game.render();
}

void timer(int value) {
    game.update();
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0); // ~60 FPS
}

void keyboard(unsigned char key, int x, int y) {
    game.handleInput(key, x, y);
}

void keyboardUp(unsigned char key, int x, int y) {
    // Handle key release if needed
}

void special(int key, int x, int y) {
    game.handleSpecialInput(key, x, y);
}

void specialUp(int key, int x, int y) {
    game.handleSpecialInputUp(key, x, y);
}

void mouse(int button, int state, int x, int y) {
    game.handleMouse(button, state, x, y);
}

void reshape(int w, int h) {
    game.reshape(w, h);
}

int main(int argc, char** argv) {
    std::ofstream logFile("debug.log");
    logFile << "Starting Egyptian Driving Game..." << std::endl;
    logFile.flush();
    
    try {
        glutInit(&argc, argv);
        logFile << "GLUT initialized" << std::endl;
        logFile.flush();
        
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
        glutInitWindowSize(800, 600);
        logFile << "Display mode and window size set" << std::endl;
        logFile.flush();
        
        int window = glutCreateWindow("Egyptian Driving Game");
        logFile << "Window created: " << window << std::endl;
        logFile.flush();

        game.init();
        logFile << "Game initialized" << std::endl;
        logFile.flush();

        glutDisplayFunc(display);
        glutTimerFunc(0, timer, 0); // Start timer immediately
        glutKeyboardFunc(keyboard);
        glutSpecialFunc(special);
        glutSpecialUpFunc(specialUp);
        
        glutMouseFunc(mouse);
        glutReshapeFunc(reshape);

        logFile << "Starting main loop..." << std::endl;
        logFile.flush();
        logFile.close();
        
        glutMainLoop();
        
    } catch (const std::exception& e) {
        logFile << "Error: " << e.what() << std::endl;
        logFile.close();
        return 1;
    }
    
    return 0;
}