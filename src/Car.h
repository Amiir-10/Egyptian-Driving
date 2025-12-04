#ifndef CAR_H
#define CAR_H

#include <GL/glut.h>

class Car {
public:
    Car();
    
    void reset(float x, float z);
    void update();
    void draw();
    
    // Controls
    void accelerate(bool on);
    void brake(bool on);
    void turnLeft(bool on);
    void turnRight(bool on);
    void toggleLights();

    // Getters
    float getX() const { return x; }
    float getZ() const { return z; }
    float getRotation() const { return rotation; }
    float getSpeed() const { return speed; }
    bool isLightsOn() const { return lightsOn; }
    
    // Setters
    void setZ(float newZ) { z = newZ; }

private:
    float x, z;
    float rotation; // Degrees
    float speed;
    float tiltAngle; // For turning effect
    
    // Physics constants
    const float MAX_SPEED = 0.3f; // Slower
    const float ACCELERATION = 0.005f; // Slower acceleration
    const float FRICTION = 0.002f;
    const float TURN_SPEED = 2.5f;
    const float MAX_TILT = 10.0f;

    // Input states
    bool isAccelerating;
    bool isBraking;
    bool isTurningLeft;
    bool isTurningRight;
    bool lightsOn;

    void drawBody();
    void drawWheels();
};

#endif
