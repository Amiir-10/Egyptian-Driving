#ifndef LEVEL_H
#define LEVEL_H

#include "Car.h"

#include "Car.h"

struct Obstacle {
    float x, z;
    float width, length;
    float speed;
    bool active;
};

class Level {
public:
    virtual ~Level() {}
    virtual void init() = 0;
    virtual void update() = 0;
    virtual void render(Car& car, bool isNight) = 0;
    virtual bool checkCollisions(Car& car) = 0;
    virtual bool isFinished(Car& car) = 0;
};

#endif
