#ifndef LEVEL1_H
#define LEVEL1_H

#include "Level.h"
#include <vector>

#include "Level.h"
#include <vector>

struct Collectible {
    float x, z;
    int type; // 0 = Traffic Light (Clear), 1 = Boost
    bool active;
    float rotation;
};

class Level1 : public Level {
public:
    Level1();
    void init() override;
    void update() override;
    void render(Car& car) override;
    bool checkCollisions(Car& car) override;
    bool isFinished(Car& car) override;

private:
    std::vector<Obstacle> cars;
    std::vector<Collectible> powerups;
    float roadLength;
    float roadWidth;
    
    void spawnCar();
    void drawRoad(float playerZ);
    void drawBuildings(float playerZ);
    void drawObstacles();
    void drawCollectibles();
};

#endif
