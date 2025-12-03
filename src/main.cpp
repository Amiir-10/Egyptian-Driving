#include <GL/glut.h>
#include "Game.h"

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
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Egyptian Driving Game");

    game.init();

    glutDisplayFunc(display);
    glutTimerFunc(0, timer, 0); // Start timer immediately
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special);
    glutSpecialUpFunc(specialUp);
    
    glutMouseFunc(mouse);
    glutReshapeFunc(reshape);

    glutMainLoop();
    return 0;
}