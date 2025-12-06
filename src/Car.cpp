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
    isTurningRight = false;
    lightsOn = true;
    boostMultiplier = 1.0f;
}

void Car::accelerate(bool on) { isAccelerating = on; }
void Car::brake(bool on) { isBraking = on; }
void Car::turnLeft(bool on) { isTurningLeft = on; }
void Car::turnRight(bool on) { isTurningRight = on; }
void Car::toggleLights() { lightsOn = !lightsOn; }
void Car::setBoost(bool on) { 
    boostMultiplier = on ? 2.0f : 1.0f; 
}

void Car::update() {
    // Speed control
    float effectiveAccel = ACCELERATION * boostMultiplier;
    float effectiveMax = MAX_SPEED * boostMultiplier;

    if (isAccelerating) {
        speed += effectiveAccel;
    } else if (isBraking) {
        speed -= effectiveAccel;
    } else {
        // Friction
        if (speed > 0) speed -= FRICTION;
        if (speed < 0) speed += FRICTION;
        if (std::abs(speed) < FRICTION) speed = 0;
    }

    // Cap speed
    if (speed > effectiveMax) speed = effectiveMax;
    if (speed < -effectiveMax / 2) speed = -effectiveMax / 2;

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
        glEnable(GL_LIGHT1);
        glEnable(GL_LIGHT2);

        // Light properties
        GLfloat lightCol[] = { 1.0f, 1.0f, 0.8f, 1.0f }; // Yellowish
        GLfloat lightAmb[] = { 0.0f, 0.0f, 0.0f, 1.0f }; // NO AMBIENT to avoid global filter effect
        
        // Spotlight properties
        GLfloat spotDir[] = { 0.0f, -0.5f, 1.0f }; // Down and forward
        GLfloat spotCutoff = 30.0f; // Narrower cone
        GLfloat spotExp = 10.0f;    // More focused towards center

        // Attenuation (Makes light fade with distance)
        // Formula: 1 / (Kc + Kl*d + Kq*d^2)
        GLfloat Kc = 1.0f;
        GLfloat Kl = 0.1f;
        GLfloat Kq = 0.05f;

        // Enable Blending for light beams
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Left Headlight
        glPushMatrix();
        glTranslatef(-0.3f, 0.0f, 0.9f); // Front Left
        
        // Visual representation (The bulb)
        glDisable(GL_LIGHTING);
        glColor3f(1.0f, 1.0f, 0.5f);
        glutSolidSphere(0.1, 10, 10);
        
        // Light Beam (Volumetric effect)
        glColor4f(1.0f, 1.0f, 0.8f, 0.2f); // Transparent yellow
        glPushMatrix();
        glRotatef(10, 1, 0, 0); // Angle down slightly (was 20, now 10)
        // Cone logic: Tip at 0, Base at Length
        // glutSolidCone draws Base at 0, Tip at Length
        // We want the opposite: Tip at 0, Base at Length
        // So we translate to Length, and scale Z by -1 to flip it back to 0
        glTranslatef(0.0f, 0.0f, 4.0f); 
        glScalef(1.0f, 1.0f, -1.0f);
        glutSolidCone(0.5, 4.0, 10, 10);
        glPopMatrix();

        glEnable(GL_LIGHTING);

        // The Light Source
        GLfloat pos1[] = { 0.0f, 0.0f, 0.0f, 1.0f }; // Relative to this pushmatrix
        glLightfv(GL_LIGHT1, GL_POSITION, pos1);
        glLightfv(GL_LIGHT1, GL_DIFFUSE, lightCol);
        glLightfv(GL_LIGHT1, GL_AMBIENT, lightAmb);
        glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, spotDir);
        glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, spotCutoff);
        glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, spotExp);
        glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, Kc);
        glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, Kl);
        glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, Kq);
        
        glPopMatrix();

        // Right Headlight
        glPushMatrix();
        glTranslatef(0.3f, 0.0f, 0.9f); // Front Right
        
        // Visual representation
        glDisable(GL_LIGHTING);
        glColor3f(1.0f, 1.0f, 0.5f);
        glutSolidSphere(0.1, 10, 10);

        // Light Beam (Volumetric effect)
        glColor4f(1.0f, 1.0f, 0.8f, 0.2f); // Transparent yellow
        glPushMatrix();
        glRotatef(10, 1, 0, 0); // Angle down slightly (was 20, now 10)
        glTranslatef(0.0f, 0.0f, 4.0f); 
        glScalef(1.0f, 1.0f, -1.0f);
        glutSolidCone(0.5, 4.0, 10, 10);
        glPopMatrix();

        glEnable(GL_LIGHTING);

        // The Light Source
        GLfloat pos2[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        glLightfv(GL_LIGHT2, GL_POSITION, pos2);
        glLightfv(GL_LIGHT2, GL_DIFFUSE, lightCol);
        glLightfv(GL_LIGHT2, GL_AMBIENT, lightAmb);
        glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, spotDir);
        glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, spotCutoff);
        glLightf(GL_LIGHT2, GL_SPOT_EXPONENT, spotExp);
        glLightf(GL_LIGHT2, GL_CONSTANT_ATTENUATION, Kc);
        glLightf(GL_LIGHT2, GL_LINEAR_ATTENUATION, Kl);
        glLightf(GL_LIGHT2, GL_QUADRATIC_ATTENUATION, Kq);

        glPopMatrix();

        glDisable(GL_BLEND);

    } else {
        glDisable(GL_LIGHT1);
        glDisable(GL_LIGHT2);
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
