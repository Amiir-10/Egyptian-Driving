#include "Level1.h"
#include <GL/glut.h>
#include <cstdlib>
#include <cmath>

Level1::Level1() {
    roadLength = 200.0f;
    roadWidth = 20.0f;
}

void Level1::init() {
    cars.clear();
    powerups.clear();
    
    // Spawn some initial cars
    for (int i = 0; i < 10; i++) {
        spawnCar();
    }

    // Spawn powerups
    for (int i = 0; i < 5; i++) {
        Collectible c;
        c.x = (rand() % (int)roadWidth) - (roadWidth / 2);
        c.z = (rand() % (int)roadLength) + 20;
        c.type = rand() % 2;
        c.active = true;
        c.rotation = 0;
        powerups.push_back(c);
    }
}

void Level1::spawnCar() {
    Obstacle car;
    car.x = (rand() % (int)roadWidth) - (roadWidth / 2);
    // Spawn ahead of the player (we need player Z, but here we just spawn relative to "current" road end?)
    // Better: Spawn relative to a "spawnZ" we track, or just far ahead.
    // For now, let's just spawn randomly in a range.
    car.z = 0; // Placeholder, set in update
    car.width = 2.0f;
    car.length = 4.0f;
    car.speed = 0.05f + ((rand() % 5) / 100.0f); // Slower speed (0.05 - 0.1)
    car.active = false; // Inactive until placed
    cars.push_back(car);
}

void Level1::update() {
    // We need player Z to generate road. 
    // But update() doesn't take player Z.
    // We should probably pass player Z to update() or store it.
    // However, we can't change the interface easily without breaking Level2.
    // Wait, we can change Level::update() signature? No, Level2 uses it.
    // Let's assume we can access playerCar from Game singleton? No.
    // Let's change Level::update(Car& car) signature in the next step.
    // For now, I'll implement the logic assuming I have 'playerZ'.
}

void Level1::render(Car& car) {
    float playerZ = car.getZ();
    
    // Infinite Road Logic
    // Draw road from [playerZ - 50] to [playerZ + 200]
    drawRoad(playerZ);
    drawBuildings(playerZ);
    
    // Update/Spawn Obstacles based on playerZ (Hack: doing logic in render or separate update)
    // Ideally logic should be in update.
    // Let's fix the update signature first.
    
    drawObstacles();
    drawCollectibles();
}

void Level1::drawRoad(float playerZ) {
    float startZ = floor(playerZ / 10.0f) * 10.0f - 50.0f;
    float endZ = startZ + 300.0f;

    // Road
    glColor3f(0.2f, 0.2f, 0.2f);
    glBegin(GL_QUADS);
    glNormal3f(0, 1, 0);
    glVertex3f(-roadWidth/2, 0.01f, startZ);
    glVertex3f(roadWidth/2, 0.01f, startZ);
    glVertex3f(roadWidth/2, 0.01f, endZ);
    glVertex3f(-roadWidth/2, 0.01f, endZ);
    glEnd();

    // Lane markings
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    for (float z = startZ; z < endZ; z += 10.0f) {
        glVertex3f(-0.2f, 0.02f, z);
        glVertex3f(0.2f, 0.02f, z);
        glVertex3f(0.2f, 0.02f, z + 5.0f);
        glVertex3f(-0.2f, 0.02f, z + 5.0f);
    }
    glEnd();
}

void Level1::drawBuildings(float playerZ) {
    float startZ = floor(playerZ / 30.0f) * 30.0f - 60.0f;
    float endZ = startZ + 300.0f;

    glColor3f(0.6f, 0.5f, 0.4f);
    for (float z = startZ; z < endZ; z += 30.0f) {
        // Left side
        glPushMatrix();
        glTranslatef(-roadWidth/2 - 10, 5.0f, z);
        glScalef(10.0f, 10.0f, 20.0f);
        glutSolidCube(1.0f);
        glPopMatrix();

        // Right side
        glPushMatrix();
        glTranslatef(roadWidth/2 + 10, 5.0f, z);
        glScalef(10.0f, 10.0f, 20.0f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }
}

void Level1::drawObstacles() {
    for (const auto& car : cars) {
        if (!car.active) continue;
        
        glPushMatrix();
        glTranslatef(car.x, 1.0f, car.z);
        
        // Car Body
        glColor3f(0.0f, 0.0f, 0.8f); // Blue cars
        glScalef(car.width, 1.5f, car.length);
        glutSolidCube(1.0f);
        
        glPopMatrix();
    }
}

void Level1::drawCollectibles() {
    for (const auto& p : powerups) {
        if (!p.active) continue;

        glPushMatrix();
        glTranslatef(p.x, 1.0f, p.z);
        glRotatef(p.rotation, 0, 1, 0);

        if (p.type == 0) { // Traffic Light
            glColor3f(1.0f, 1.0f, 0.0f);
            glutSolidCube(1.0f);
        } else { // Boost
            glColor3f(0.0f, 1.0f, 1.0f);
            glutSolidCone(0.5f, 1.0f, 10, 2);
        }

        glPopMatrix();
    }
}

bool Level1::checkCollisions(Car& car) {
    float carX = car.getX();
    float carZ = car.getZ();
    float carSize = 1.0f; // Approx radius

    // Manage Obstacles (Spawn/Despawn)
    // This is a hack to do it in collision check, but we have access to 'car' here!
    // Ideally we should update the Level::update signature.
    
    // Spawn new cars ahead
    for (auto& obs : cars) {
        if (!obs.active) {
            if (rand() % 100 < 2) { // Chance to spawn
                obs.active = true;
                obs.x = (rand() % (int)roadWidth) - (roadWidth / 2);
                obs.z = carZ + 100 + (rand() % 50);
                obs.speed = 0.05f + ((rand() % 5) / 100.0f);
            }
        } else {
            // Move car
            obs.z += obs.speed; // Move forward (or backward relative to player?)
            // If traffic is moving same direction:
            // obs.z += obs.speed;
            
            // Despawn if too far behind
            if (obs.z < carZ - 20) {
                obs.active = false;
            }
        }
    }

    // Check obstacles
    for (auto& obs : cars) {
        if (obs.active) {
            if (std::abs(carX - obs.x) < (carSize + obs.width/2) &&
                std::abs(carZ - obs.z) < (carSize + obs.length/2)) {
                return true; // Collision!
            }
        }
    }

    // Check collectibles
    for (auto& p : powerups) {
        // Respawn logic for powerups
        if (!p.active && rand() % 200 < 1) {
            p.active = true;
            p.x = (rand() % (int)roadWidth) - (roadWidth / 2);
            p.z = carZ + 150 + (rand() % 50);
        }
        
        if (p.active) {
            if (p.z < carZ - 20) p.active = false; // Despawn

            if (std::abs(carX - p.x) < 1.5f && std::abs(carZ - p.z) < 1.5f) {
                p.active = false;
                if (p.type == 1) car.accelerate(true); // Boost hack (doesn't really work with bool)
                // We need a way to boost speed directly.
            }
        }
    }

    return false;
}



bool Level1::isFinished(Car& car) {
    return car.getZ() > 1000.0f;
}
