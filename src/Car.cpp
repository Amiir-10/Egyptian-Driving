#include "Car.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Car::Car() {
    reset(0, 0);
}

void Car::reset(float startX, float startZ) {
    x = startX;
    z = startZ;
    rotation = 0.0f;
    speed = 0.0f;
    tiltAngle = 0.0f;
    isAccelerating = false;
    isBraking = false;
    isTurningLeft = false;
    isTurningRight = false;
    lightsOn = true;
}

void Car::accelerate(bool on) { isAccelerating = on; }
void Car::brake(bool on) { isBraking = on; }
void Car::turnLeft(bool on) { isTurningLeft = on; }
void Car::turnRight(bool on) { isTurningRight = on; }
void Car::toggleLights() { lightsOn = !lightsOn; }

void Car::update() {
    // Speed control
    if (isAccelerating) {
        speed += ACCELERATION;
    } else if (isBraking) {
        speed -= ACCELERATION;
    } else {
        // Friction
        if (speed > 0) speed -= FRICTION;
        if (speed < 0) speed += FRICTION;
        if (std::abs(speed) < FRICTION) speed = 0;
    }

    // Cap speed
    if (speed > MAX_SPEED) speed = MAX_SPEED;
    if (speed < -MAX_SPEED / 2) speed = -MAX_SPEED / 2;

    // Turning (Allow turning even if slow, but maybe less?)
    // User said "I have no way to steer". Maybe they were stopped?
    // Let's allow steering if speed is non-zero OR if accelerating (burnout style?)
    // For now, keep speed check but ensure TURN_SPEED is effective.
    if (std::abs(speed) > 0.001f) {
        float turnFactor = (speed > 0) ? 1.0f : -1.0f;
        if (isTurningLeft) rotation += TURN_SPEED * turnFactor;
        if (isTurningRight) rotation -= TURN_SPEED * turnFactor;
    }

    // Tilt Logic
    float targetTilt = 0.0f;
    if (isTurningLeft) targetTilt = -MAX_TILT;
    if (isTurningRight) targetTilt = MAX_TILT;
    
    // Interpolate tilt
    if (tiltAngle < targetTilt) tiltAngle += 1.0f;
    if (tiltAngle > targetTilt) tiltAngle -= 1.0f;

    // Update position
    float rad = rotation * M_PI / 180.0f;
    x += sin(rad) * speed;
    z += cos(rad) * speed;
}

void Car::draw() {
    glPushMatrix();
    glTranslatef(x, 0.5f, z); // Lift car slightly above ground
    glRotatef(rotation, 0, 1, 0);
    glRotatef(tiltAngle, 0, 0, 1); // Apply tilt (Roll)

    drawBody();
    drawWheels();

    // Headlights
    if (lightsOn) {
        // Setup spotlight logic here or in drawBody
        // For now, just visual cones
        glPushMatrix();
        glTranslatef(0.3f, 0.0f, 1.0f);
        glColor3f(1.0f, 1.0f, 0.8f);
        // glutSolidCone(0.1, 0.5, 10, 2); // Visual representation
        glPopMatrix();
    }

    glPopMatrix();
}

void Car::drawBody() {
    // Main body
    glColor3f(0.8f, 0.1f, 0.1f); // Red car
    glPushMatrix();
    glScalef(0.8f, 0.4f, 1.8f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Roof
    glColor3f(0.7f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.0f, 0.4f, -0.2f);
    glScalef(0.7f, 0.3f, 1.0f);
    glutSolidCube(1.0f);
    glPopMatrix();
}

void Car::drawWheels() {
    glColor3f(0.1f, 0.1f, 0.1f); // Black wheels
    
    float wheelX = 0.45f;
    float wheelY = -0.2f;
    float wheelZ = 0.5f;
    float wheelSize = 0.25f;

    // Front Left
    glPushMatrix();
    glTranslatef(wheelX, wheelY, wheelZ);
    glRotatef(90, 0, 1, 0);
    glutSolidTorus(0.05, wheelSize, 10, 20);
    glPopMatrix();

    // Front Right
    glPushMatrix();
    glTranslatef(-wheelX, wheelY, wheelZ);
    glRotatef(90, 0, 1, 0);
    glutSolidTorus(0.05, wheelSize, 10, 20);
    glPopMatrix();

    // Rear Left
    glPushMatrix();
    glTranslatef(wheelX, wheelY, -wheelZ);
    glRotatef(90, 0, 1, 0);
    glutSolidTorus(0.05, wheelSize, 10, 20);
    glPopMatrix();

    // Rear Right
    glPushMatrix();
    glTranslatef(-wheelX, wheelY, -wheelZ);
    glRotatef(90, 0, 1, 0);
    glutSolidTorus(0.05, wheelSize, 10, 20);
    glPopMatrix();
}
