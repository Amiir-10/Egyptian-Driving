#include "Level2.h"
#include <GL/glut.h>
#include <cmath>
#include <iostream>

Level2::Level2() {
    parked = false;
}

void Level2::init() {
    obstacles.clear();
    
    // Target Spot
    targetSpot.x = 10.0f;
    targetSpot.z = 20.0f;
    targetSpot.width = 3.0f;
    targetSpot.length = 5.0f;

    // Add Cones around
    Obstacle cone;
    cone.width = 0.5f;
    cone.length = 0.5f;
    cone.active = true;
    
    // Place cones
    float positions[][2] = {{5, 15}, {15, 15}, {5, 25}, {15, 25}};
    for (auto& pos : positions) {
        cone.x = pos[0];
        cone.z = pos[1];
        obstacles.push_back(cone);
    }

    // Add Sayes
    Obstacle sayes;
    sayes.x = 8.0f;
    sayes.z = 22.0f;
    sayes.width = 0.8f;
    sayes.length = 0.8f;
    sayes.active = true; // Mark as Sayes type if needed, or just generic obstacle
    obstacles.push_back(sayes);
}

void Level2::update() {
    // Static obstacles, nothing to update really
    // Maybe animate Sayes waving?
}

void Level2::render(Car& car) {
    drawParkingLot();
    drawCones();
    drawSayes();
    drawMirror(car);
}

void Level2::drawParkingLot() {
    // Asphalt
    glColor3f(0.2f, 0.2f, 0.2f);
    glBegin(GL_QUADS);
    glNormal3f(0, 1, 0);
    glVertex3f(-50, 0.01f, -50);
    glVertex3f(50, 0.01f, -50);
    glVertex3f(50, 0.01f, 50);
    glVertex3f(-50, 0.01f, 50);
    glEnd();

    // Target Spot markings
    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(3.0f);
    glBegin(GL_LINE_LOOP);
    glVertex3f(targetSpot.x - targetSpot.width/2, 0.02f, targetSpot.z - targetSpot.length/2);
    glVertex3f(targetSpot.x + targetSpot.width/2, 0.02f, targetSpot.z - targetSpot.length/2);
    glVertex3f(targetSpot.x + targetSpot.width/2, 0.02f, targetSpot.z + targetSpot.length/2);
    glVertex3f(targetSpot.x - targetSpot.width/2, 0.02f, targetSpot.z + targetSpot.length/2);
    glEnd();
}

void Level2::drawCones() {
    glColor3f(1.0f, 0.5f, 0.0f); // Orange
    for (const auto& obs : obstacles) {
        // Simple check if it's a cone (based on size/index)
        // For now draw all as cones except Sayes
        if (obs.x == 8.0f) continue; // Skip Sayes

        glPushMatrix();
        glTranslatef(obs.x, 0.0f, obs.z);
        glRotatef(-90, 1, 0, 0);
        glutSolidCone(0.3f, 1.0f, 10, 2);
        glPopMatrix();
    }
}

void Level2::drawSayes() {
    // Draw Sayes at 8, 22
    glPushMatrix();
    glTranslatef(8.0f, 0.0f, 22.0f);
    
    // Body
    glColor3f(0.0f, 0.0f, 1.0f);
    glPushMatrix();
    glTranslatef(0.0f, 0.75f, 0.0f);
    glScalef(0.4f, 1.5f, 0.2f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Head
    glColor3f(1.0f, 0.8f, 0.6f);
    glPushMatrix();
    glTranslatef(0.0f, 1.6f, 0.0f);
    glutSolidSphere(0.25f, 10, 10);
    glPopMatrix();

    glPopMatrix();
}

void Level2::drawMirror(Car& car) {
    // Simple rear view mirror simulation
    // In GLUT, we can't easily do render-to-texture without extensions or FBOs manually.
    // We can use glViewport to draw a small view at the top.
    
    // Save current attributes
    glPushAttrib(GL_VIEWPORT_BIT | GL_ENABLE_BIT);
    glDisable(GL_LIGHTING);
    
    // Set viewport to top center
    int w = glutGet(GLUT_WINDOW_WIDTH);
    int h = glutGet(GLUT_WINDOW_HEIGHT);
    glViewport(w/2 - 100, h - 100, 200, 80);
    
    // Clear depth for this viewport so it draws on top
    glClear(GL_DEPTH_BUFFER_BIT);
    
    // Setup camera looking BACKWARDS
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluPerspective(45.0f, 2.5f, 0.1f, 100.0f);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    // Get player position
    float carX = car.getX();
    float carZ = car.getZ();
    float carRot = car.getRotation() * 3.14159f / 180.0f;
    
    // Camera at car position, looking back
    float eyeX = carX;
    float eyeY = 1.5f;
    float eyeZ = carZ;
    
    float lookX = carX - sin(carRot) * 10.0f; // Look behind (opposite to forward)
    float lookZ = carZ - cos(carRot) * 10.0f;
    
    gluLookAt(eyeX, eyeY, eyeZ, lookX, 1.0f, lookZ, 0.0f, 1.0f, 0.0f);
    
    // Draw scene again (simplified)
    drawParkingLot();
    drawCones();
    drawSayes();
    
    // Draw frame
    // glColor3f(0.5f, 0.5f, 0.5f);
    // ...
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    
    glPopAttrib();
}

bool Level2::checkCollisions(Car& car) {
    float carX = car.getX();
    float carZ = car.getZ();
    float carSize = 1.0f;

    for (const auto& obs : obstacles) {
        if (std::abs(carX - obs.x) < (carSize + obs.width/2) &&
            std::abs(carZ - obs.z) < (carSize + obs.length/2)) {
            return true;
        }
    }
    
    // Check parking
    if (std::abs(carX - targetSpot.x) < 0.5f && 
        std::abs(carZ - targetSpot.z) < 0.5f &&
        std::abs(car.getSpeed()) < 0.01f) {
        parked = true;
    }

    return false;
}

bool Level2::isFinished(Car& car) {
    return parked;
}
