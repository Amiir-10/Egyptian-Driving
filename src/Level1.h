#ifndef LEVEL1_H
#define LEVEL1_H

#include "Level.h"
#include "Model_3DS.h"
#include <vector>

struct Collectible
{
    float x, z;
    int type; // 0 = Traffic Light (Clear), 1 = Boost
    bool active;
    float rotation;
};

class Level1 : public Level
{
public:
    Level1();
    void init() override;
    void update() override;
    void render(Car &car, bool isNight) override;
    bool checkCollisions(Car &car) override;
    bool isFinished(Car &car) override;

private:
    std::vector<Obstacle> cars;
    std::vector<Collectible> powerups;
    float roadLength;
    float roadWidth;
    bool wasLightsOn;

    float noTrafficTimer;
    bool noTrafficActive;

    float speedBoostTimer;
    bool speedBoostActive;
    float animationTime; // For scaling animation

    // Obstacle car 3D model
    Model_3DS obstacleCarModel;
    bool obstacleModelLoaded;

    // No Traffic power-up 3D model
    Model_3DS noTrafficModel;
    bool noTrafficModelLoaded;

    void spawnCar();
    void drawRoad(float playerZ);
    void drawGround(float playerZ);
    void drawBuildings(float playerZ);
    void drawLampPosts(float playerZ, bool isNight);
    void drawObstacles();
    void drawCollectibles();
};

#endif
