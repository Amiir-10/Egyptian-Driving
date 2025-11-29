#ifndef LEVEL2_H
#define LEVEL2_H

#include "Level.h"
#include <vector>

struct ParkingSpot {
    float x, z;
    float width, length;
};

class Level2 : public Level {
public:
    Level2();
    void init() override;
    void update() override;
    void render(Car& car) override;
    bool checkCollisions(Car& car) override;
    bool isFinished(Car& car) override;

private:
    std::vector<Obstacle> obstacles; // Cones, cars, Sayes
    ParkingSpot targetSpot;
    bool parked;
    
    void drawParkingLot();
    void drawCones();
    void drawSayes();
    void drawMirror(Car& car);
};

#endif
