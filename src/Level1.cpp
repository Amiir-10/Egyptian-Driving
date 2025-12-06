#include "Level1.h"
#include <GL/glut.h>
#include <cstdlib>
#include <cmath>
#include <cstdio>

Level1::Level1()
{
    roadLength = 200.0f;
    roadWidth = 20.0f;
    wasLightsOn = false;
    noTrafficTimer = 0.0f;
    noTrafficActive = false;
    speedBoostTimer = 0.0f;
    speedBoostActive = false;
    animationTime = 0.0f;
}

void Level1::init()
{
    cars.clear();
    powerups.clear();

    noTrafficTimer = 0.0f;
    noTrafficActive = false;
    speedBoostTimer = 0.0f;
    speedBoostActive = false;

    // Spawn some initial cars
    for (int i = 0; i < 10; i++)
    {
        spawnCar();
    }

    // Spawn powerups
    for (int i = 0; i < 5; i++)
    {
        Collectible c;
        c.x = ((rand() % 16) - 8.0f); // Strictly on road (-8 to 8)
        c.z = (rand() % (int)roadLength) + 20;
        // 75% chance for speed boost (type 1), 25% for traffic light (type 0)
        c.type = (rand() % 100 < 75) ? 1 : 0;
        c.active = true;
        c.rotation = 0;
        c.animationTime = 0;
        powerups.push_back(c);
    }
}

void Level1::spawnCar()
{
    Obstacle car;
    car.x = (rand() % (int)roadWidth) - (roadWidth / 2);
    // Spawn ahead of the player (we need player Z, but here we just spawn relative to "current" road end?)
    // Better: Spawn relative to a "spawnZ" we track, or just far ahead.
    // For now, let's just spawn randomly in a range.
    car.z = 0; // Placeholder, set in update
    car.width = 2.0f;
    car.length = 4.0f;
    car.speed = 0.05f + ((rand() % 5) / 100.0f); // Slower speed (0.05 - 0.1)
    car.active = false;                          // Inactive until placed

    car.targetX = 0;
    car.isMovingAside = false;
    car.originalX = 0;

    cars.push_back(car);
}

void Level1::update()
{
    // We need player Z to generate road.
    // But update() doesn't take player Z.
    // We should probably pass player Z to update() or store it.
    // However, we can't change the interface easily without breaking Level2.
    // Wait, we can change Level::update() signature? No, Level2 uses it.
    // Let's assume we can access playerCar from Game singleton? No.
    // Let's change Level::update(Car& car) signature in the next step.
    // For now, I'll implement the logic assuming I have 'playerZ'.
}

void Level1::render(Car &car, bool isNight)
{
    float playerZ = car.getZ();

    // Infinite Road Logic
    // Draw road from [playerZ - 50] to [playerZ + 200]
    drawRoad(playerZ);
    drawGround(playerZ);
    drawBuildings(playerZ);
    drawLampPosts(playerZ, isNight);

    // Update/Spawn Obstacles based on playerZ (Hack: doing logic in render or separate update)
    // Ideally logic should be in update.
    // Let's fix the update signature first.
    drawLampPosts(playerZ, isNight);

    // Update/Spawn Obstacles based on playerZ (Hack: doing logic in render or separate update)
    // Ideally logic should be in update.
    // Let's fix the update signature first.

    drawObstacles();
    drawCollectibles();

    // Update animation time (approx 60 FPS)
    animationTime += 0.05f;

    // Draw No Traffic Timer
    if (noTrafficActive)
    {
        glDisable(GL_LIGHTING);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0, 800, 0, 600);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glColor3f(1.0f, 1.0f, 0.0f); // Yellow
        // Format time to 1 decimal place
        char timeBuffer[32];
        sprintf(timeBuffer, "No Traffic: %.1fs", noTrafficTimer);
        // std::string timeStr = "No Traffic: " + std::to_string(noTrafficTimer).substr(0, 3) + "s";

        glRasterPos2f(300, 550); // Top center-ish
        for (char *c = timeBuffer; *c != '\0'; c++)
        {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        }

        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glEnable(GL_LIGHTING);
    }

    // Draw Speed Boost Timer
    if (speedBoostActive)
    {
        glDisable(GL_LIGHTING);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0, 800, 0, 600);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glColor3f(0.0f, 1.0f, 1.0f); // Cyan
        char timeBuffer[32];
        sprintf(timeBuffer, "Speed Boost: %.1fs", speedBoostTimer);

        glRasterPos2f(300, 520); // Slightly below No Traffic
        for (char *c = timeBuffer; *c != '\0'; c++)
        {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        }

        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glEnable(GL_LIGHTING);
    }
}

void Level1::drawRoad(float playerZ)
{
    float startZ = floor(playerZ / 10.0f) * 10.0f - 50.0f;
    float endZ = startZ + 300.0f;

    // Road
    glColor3f(0.2f, 0.2f, 0.2f);
    glBegin(GL_QUADS);
    glNormal3f(0, 1, 0);
    glVertex3f(-roadWidth / 2, 0.01f, startZ);
    glVertex3f(roadWidth / 2, 0.01f, startZ);
    glVertex3f(roadWidth / 2, 0.01f, endZ);
    glVertex3f(-roadWidth / 2, 0.01f, endZ);
    glEnd();

    // Lane markings
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    for (float z = startZ; z < endZ; z += 10.0f)
    {
        glVertex3f(-0.2f, 0.02f, z);
        glVertex3f(0.2f, 0.02f, z);
        glVertex3f(0.2f, 0.02f, z + 5.0f);
        glVertex3f(-0.2f, 0.02f, z + 5.0f);
    }
    glEnd();
}

void Level1::drawBuildings(float playerZ)
{
    float startZ = floor(playerZ / 30.0f) * 30.0f - 60.0f;
    float endZ = startZ + 300.0f;

    glColor3f(0.6f, 0.5f, 0.4f);
    for (float z = startZ; z < endZ; z += 30.0f)
    {
        // Left side
        glPushMatrix();
        glTranslatef(-roadWidth / 2 - 10, 5.0f, z);
        glScalef(10.0f, 10.0f, 20.0f);
        glutSolidCube(1.0f);
        glPopMatrix();

        // Right side
        glPushMatrix();
        glTranslatef(roadWidth / 2 + 10, 5.0f, z);
        glScalef(10.0f, 10.0f, 20.0f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }
}

void Level1::drawObstacles()
{
    for (const auto &car : cars)
    {
        if (!car.active)
            continue;

        glPushMatrix();
        glTranslatef(car.x, 1.0f, car.z);

        // Car Body
        glColor3f(0.0f, 0.0f, 0.8f); // Blue cars
        glScalef(car.width, 1.5f, car.length);
        glutSolidCube(1.0f);

        glPopMatrix();
    }
}

void Level1::drawCollectibles()
{
    for (const auto &p : powerups)
    {
        if (!p.active)
            continue;

        // Update animation time
        p.animationTime += 0.05f;
        p.rotation += 2.0f; // Rotate for visibility
        
        // Calculate bobbing effect (up and down)
        float bobHeight = sin(p.animationTime) * 0.3f;
        
        // Calculate pulsing scale
        float pulseScale = 1.0f + sin(p.animationTime * 2.0f) * 0.15f;

        glPushMatrix();
        glTranslatef(p.x, 1.5f + bobHeight, p.z); // Higher position with bobbing
        glRotatef(p.rotation, 0, 1, 0);

        if (p.type == 0)
        { // Traffic Light
            glColor3f(1.0f, 1.0f, 0.0f);
            // Pulsating Animation
            float scale = 1.0f + 0.2f * sin(animationTime);
            glScalef(scale, scale, scale);
            glutSolidCube(1.0f);
        }
        else
        { // Boost
            glColor3f(0.0f, 1.0f, 1.0f);
            // Floating Animation
            float offset = 0.5f * sin(animationTime);
            glTranslatef(0.0f, offset, 0.0f);
            glutSolidCone(0.5f, 1.0f, 10, 2);
        }

        glPopMatrix();
    }
}

bool Level1::checkCollisions(Car &car)
{
    float carX = car.getX();
    float carZ = car.getZ();
    float carSize = 1.0f; // Approx radius

    // Handle No Traffic Timer
    if (noTrafficActive)
    {
        noTrafficTimer -= 0.016f; // Approx 60 FPS
        if (noTrafficTimer <= 0.0f)
        {
            noTrafficActive = false;
        }
        else
        {
            // Despawn all cars while active
            for (auto &obs : cars)
            {
                obs.active = false;
            }
        }
    }

    // Handle Speed Boost Timer
    if (speedBoostActive)
    {
        speedBoostTimer -= 0.016f;
        if (speedBoostTimer <= 0.0f)
        {
            speedBoostActive = false;
            car.setBoost(false);
        }
    }

    // Manage Obstacles (Spawn/Despawn)
    // Only spawn if traffic is allowed
    if (!noTrafficActive)
    {
        // Spawn new cars ahead
        for (auto &obs : cars)
        {
            if (!obs.active)
            {
                if (rand() % 100 < 2)
                { // Chance to spawn
                    // Spawn strictly on road (width 20, so -10 to 10). Keep away from edges.
                    // Range: -8 to 8
                    float newX = (rand() % 16) - 8.0f;
                    float newZ = carZ + 100 + (rand() % 50);

                    // Check overlap with existing active cars
                    bool overlap = false;
                    for (const auto &other : cars)
                    {
                        if (other.active)
                        {
                            if (std::abs(newX - other.x) < 4.0f && std::abs(newZ - other.z) < 10.0f)
                            {
                                overlap = true;
                                break;
                            }
                        }
                    }

                    if (!overlap)
                    {
                        obs.active = true;
                        obs.x = newX;
                        obs.z = newZ;
                        obs.speed = 0.05f + ((rand() % 5) / 100.0f);

                        obs.originalX = obs.x;
                        obs.targetX = obs.x;
                        obs.isMovingAside = false;
                    }
                }
            }
            else
            {
                // Move car
                obs.z += obs.speed;

                // Smart Behavior: Move aside if illuminated (Flash Trigger)
                bool lightsOn = car.isLightsOn();
                bool justFlashed = lightsOn && !wasLightsOn;

                if (justFlashed)
                {
                    // Check if in front and within REDUCED range
                    float distZ = obs.z - carZ;
                    if (distZ > 0 && distZ < 15.0f)
                    { // Reduced range
                        // Check if in same lane (roughly)
                        if (std::abs(obs.x - carX) < 4.0f)
                        {
                            obs.isMovingAside = true;
                            // Decide direction: Move away from center or just to shoulder
                            if (obs.x > 0)
                                obs.targetX = obs.x + 6.0f;
                            else
                                obs.targetX = obs.x - 6.0f;
                        }
                    }
                }

                // Interpolate position (SLOWER)
                if (obs.isMovingAside)
                {
                    obs.x += (obs.targetX - obs.x) * 0.01f; // Reduced from 0.05 to 0.01
                }

                // Despawn if too far behind OR if on grass
                // Road width is 20 (-10 to 10). If |x| > 10, it's on grass.
                if (obs.z < carZ - 20 || std::abs(obs.x) > 10.0f)
                {
                    obs.active = false;
                }
            }
        }
    }

    // Update light state for next frame
    wasLightsOn = car.isLightsOn();

    // Check obstacles
    for (auto &obs : cars)
    {
        if (obs.active)
        {
            if (std::abs(carX - obs.x) < (carSize + obs.width / 2) &&
                std::abs(carZ - obs.z) < (carSize + obs.length / 2))
            {
                return true; // Collision!
            }
        }
    }

    // Check collectibles
    for (auto &p : powerups)
    {
        // Respawn logic for powerups - Reduced frequency and overlap check
        if (!p.active && rand() % 500 < 1)
        {                                      // Reduced frequency (was 200)
            float newX = (rand() % 16) - 8.0f; // On road
            float newZ = carZ + 150 + (rand() % 50);

            // Check overlap with other powerups
            bool overlap = false;
            for (const auto &other : powerups)
            {
                if (other.active)
                {
                    if (std::abs(newX - other.x) < 2.0f && std::abs(newZ - other.z) < 2.0f)
                    {
                        overlap = true;
                        break;
                    }
                }
            }

            if (!overlap)
            {
                p.active = true;
                p.x = newX;
                p.z = newZ;
                p.type = rand() % 2; // Randomly assign type on respawn (0 = No Traffic, 1 = Speed Boost)
            }
        }

        if (p.active)
        {
            if (p.z < carZ - 20)
                p.active = false; // Despawn

            if (std::abs(carX - p.x) < 1.5f && std::abs(carZ - p.z) < 1.5f)
            {
                p.active = false;
                if (p.type == 0)
                { // Traffic Light (No Traffic)
                    noTrafficActive = true;
                    noTrafficTimer = 5.0f; // 5 seconds
                }
                else if (p.type == 1)
                { // Boost
                    speedBoostActive = true;
                    speedBoostTimer = 3.0f;
                    car.setBoost(true);
                }
            }
        }
    }

    // Check Lamp Posts
    // Lamp posts are at +/- (roadWidth/2 + 2.0f) = +/- 12.0f
    // Spaced every 30.0f, offset by 15.0f
    // Formula: z = k * 30 + 15
    // We can check if car is near the sides first
    if (std::abs(carX) > roadWidth / 2 + 1.0f)
    { // Near sides
        // Check Z alignment with posts
        // Normalize Z to find nearest post
        // z - 15 = k * 30
        // (z - 15) / 30 = k
        float localZ = carZ - 15.0f;
        float nearestK = round(localZ / 30.0f);
        float nearestPostZ = nearestK * 30.0f + 15.0f;

        if (std::abs(carZ - nearestPostZ) < 1.0f)
        {                // Close to post Z
            return true; // Collision with lamp post
        }
    }

    // Check Buildings
    // Buildings are at +/- (roadWidth/2 + 10) = +/- 20.0f
    // Width (X) is 10.0f, so they span [15, 25] and [-25, -15].
    // Length (Z) is 20.0f. Spaced every 30.0f.
    // Centered at k * 30. Spans [k*30 - 10, k*30 + 10].

    // Check X first (simple bounding box)
    // Car width is approx 2.0f.
    // If carX > 15 - 1 = 14 (Right side) or carX < -14 (Left side)
    if (std::abs(carX) > 14.0f)
    {
        // Check Z
        // Normalize Z to nearest building center
        // Building centers are at multiples of 30.
        float localZ = carZ;
        float nearestK = round(localZ / 30.0f);
        float nearestBuildingZ = nearestK * 30.0f;

        // Check if within length of building (20 total, so +/- 10)
        // Add car length (approx 4.0f, so +/- 2.0f)
        if (std::abs(carZ - nearestBuildingZ) < 10.0f + 2.0f)
        {
            return true; // Collision with building
        }
    }

    return false;
}

bool Level1::isFinished(Car &car)
{
    return car.getZ() > 1000.0f;
}

void Level1::drawGround(float playerZ)
{
    float startZ = floor(playerZ / 10.0f) * 10.0f - 50.0f;
    float endZ = startZ + 300.0f;
    float groundWidth = 100.0f; // Width of the grass strips

    glColor3f(0.0f, 0.8f, 0.0f); // Green Grass
    glNormal3f(0, 1, 0);

    // Left Side
    glBegin(GL_QUADS);
    glVertex3f(-roadWidth / 2 - groundWidth, 0.01f, startZ);
    glVertex3f(-roadWidth / 2, 0.01f, startZ);
    glVertex3f(-roadWidth / 2, 0.01f, endZ);
    glVertex3f(-roadWidth / 2 - groundWidth, 0.01f, endZ);
    glEnd();

    // Right Side
    glBegin(GL_QUADS);
    glVertex3f(roadWidth / 2, 0.01f, startZ);
    glVertex3f(roadWidth / 2 + groundWidth, 0.01f, startZ);
    glVertex3f(roadWidth / 2 + groundWidth, 0.01f, endZ);
    glVertex3f(roadWidth / 2, 0.01f, endZ);
    glEnd();
}

void Level1::drawLampPosts(float playerZ, bool isNight)
{
    float startZ = floor(playerZ / 30.0f) * 30.0f - 60.0f;
    float endZ = startZ + 300.0f;

    for (float z = startZ; z < endZ; z += 30.0f)
    {
        // Left Side
        glPushMatrix();
        glTranslatef(-roadWidth / 2 - 2.0f, 0.0f, z + 15.0f); // Offset from buildings

        // Pole
        glColor3f(0.3f, 0.3f, 0.3f); // Gray
        glPushMatrix();
        glRotatef(-90, 1, 0, 0);
        GLUquadricObj *qobj = gluNewQuadric();
        gluCylinder(qobj, 0.3, 0.3, 6.0, 10, 10);
        gluDeleteQuadric(qobj);
        glPopMatrix();

        // Arm
        glPushMatrix();
        glTranslatef(0.0f, 6.0f, 0.0f);
        glRotatef(90, 0, 1, 0); // Point towards road
        qobj = gluNewQuadric();
        gluCylinder(qobj, 0.2, 0.2, 3.0, 10, 10);
        gluDeleteQuadric(qobj);
        glPopMatrix();

        // Lamp
        glPushMatrix();
        glTranslatef(3.0f, 5.8f, 0.0f); // End of arm
        glColor3f(1.0f, 1.0f, 1.0f);
        glutSolidSphere(0.4, 10, 10);

        // Light Beam (Night only)
        if (isNight)
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDepthMask(GL_FALSE);              // Don't write to depth buffer for transparent objects
            glColor4f(1.0f, 1.0f, 0.8f, 0.15f); // More transparent (was 0.3)

            glPushMatrix();
            glRotatef(90, 1, 0, 0); // Point down
            // Flip cone: Tip at 0, Base at Length
            glTranslatef(0.0f, 0.0f, 6.0f);
            glScalef(1.0f, 1.0f, -1.0f);
            glutSolidCone(2.0, 6.0, 10, 10); // Wide cone down to ground
            glPopMatrix();

            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND);
        }
        glPopMatrix();
        glPopMatrix();

        // Right Side
        glPushMatrix();
        glTranslatef(roadWidth / 2 + 2.0f, 0.0f, z + 15.0f);

        // Pole
        glColor3f(0.3f, 0.3f, 0.3f);
        glPushMatrix();
        glRotatef(-90, 1, 0, 0);
        qobj = gluNewQuadric();
        gluCylinder(qobj, 0.3, 0.3, 6.0, 10, 10);
        gluDeleteQuadric(qobj);
        glPopMatrix();

        // Arm
        glPushMatrix();
        glTranslatef(0.0f, 6.0f, 0.0f);
        glRotatef(-90, 0, 1, 0); // Point towards road (negative)
        qobj = gluNewQuadric();
        gluCylinder(qobj, 0.2, 0.2, 3.0, 10, 10);
        gluDeleteQuadric(qobj);
        glPopMatrix();

        // Lamp
        glPushMatrix();
        glTranslatef(-3.0f, 5.8f, 0.0f);
        glColor3f(1.0f, 1.0f, 1.0f);
        glutSolidSphere(0.4, 10, 10);

        // Light Beam (Night only)
        if (isNight)
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDepthMask(GL_FALSE);
            glColor4f(1.0f, 1.0f, 0.8f, 0.15f);

            glPushMatrix();
            glRotatef(90, 1, 0, 0);
            glTranslatef(0.0f, 0.0f, 6.0f);
            glScalef(1.0f, 1.0f, -1.0f);
            glutSolidCone(2.0, 6.0, 10, 10);
            glPopMatrix();

            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND);
        }
        glPopMatrix();
        glPopMatrix();
    }
}

void Level1::drawTrafficLight(float scale) {
    glPushMatrix();
    glScalef(scale, scale, scale);
    
    // Pole (black)
    glColor3f(0.1f, 0.1f, 0.1f);
    glPushMatrix();
    glTranslatef(0.0f, -0.5f, 0.0f);
    glRotatef(-90, 1, 0, 0);
    GLUquadricObj *qobj = gluNewQuadric();
    gluCylinder(qobj, 0.1, 0.1, 1.0, 10, 10);
    gluDeleteQuadric(qobj);
    glPopMatrix();
    
    // Traffic light housing (dark gray)
    glColor3f(0.2f, 0.2f, 0.2f);
    glPushMatrix();
    glScalef(0.4f, 0.9f, 0.3f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    // Red light (top)
    glPushMatrix();
    glTranslatef(0.0f, 0.25f, 0.16f);
    glColor3f(0.5f, 0.0f, 0.0f); // Dim red
    glutSolidSphere(0.12f, 10, 10);
    glPopMatrix();
    
    // Yellow light (middle) - GLOWING
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.16f);
    
    // Outer glow
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glColor4f(1.0f, 1.0f, 0.0f, 0.3f);
    glutSolidSphere(0.18f, 10, 10);
    glDisable(GL_BLEND);
    
    // Bright core
    glColor3f(1.0f, 1.0f, 0.0f);
    glutSolidSphere(0.12f, 10, 10);
    glPopMatrix();
    
    // Green light (bottom)
    glPushMatrix();
    glTranslatef(0.0f, -0.25f, 0.16f);
    glColor3f(0.0f, 0.3f, 0.0f); // Dim green
    glutSolidSphere(0.12f, 10, 10);
    glPopMatrix();
    
    glPopMatrix();
}

void Level1::drawSpeedBoost(float scale) {
    glPushMatrix();
    glScalef(scale, scale, scale);
    
    // Double arrow pointing upwards/forwards
    
    // Base platform (glowing cyan)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glColor4f(0.0f, 1.0f, 1.0f, 0.4f);
    glPushMatrix();
    glTranslatef(0.0f, -0.3f, 0.0f);
    glScalef(0.8f, 0.1f, 0.8f);
    glutSolidCube(1.0f);
    glPopMatrix();
    glDisable(GL_BLEND);
    
    // First arrow (bottom)
    glColor3f(0.0f, 1.0f, 1.0f); // Bright cyan
    
    // Arrow shaft
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.0f);
    glScalef(0.15f, 0.5f, 0.15f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    // Arrow head (pyramid)
    glPushMatrix();
    glTranslatef(0.0f, 0.35f, 0.0f);
    glRotatef(-90, 1, 0, 0);
    glutSolidCone(0.3f, 0.4f, 4, 1);
    glPopMatrix();
    
    // Second arrow (top) - slightly offset
    glColor3f(0.5f, 1.0f, 1.0f); // Lighter cyan
    
    // Arrow shaft
    glPushMatrix();
    glTranslatef(0.0f, 0.5f, 0.0f);
    glScalef(0.12f, 0.4f, 0.12f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    // Arrow head (pyramid)
    glPushMatrix();
    glTranslatef(0.0f, 0.8f, 0.0f);
    glRotatef(-90, 1, 0, 0);
    glutSolidCone(0.25f, 0.35f, 4, 1);
    glPopMatrix();
    
    // Add sparkle effect with small rotating cubes
    glPushMatrix();
    glRotatef(scale * 100.0f, 0, 1, 0);
    for (int i = 0; i < 4; i++) {
        glPushMatrix();
        glRotatef(i * 90, 0, 1, 0);
        glTranslatef(0.5f, 0.4f, 0.0f);
        glColor3f(1.0f, 1.0f, 1.0f);
        glScalef(0.05f, 0.05f, 0.05f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }
    glPopMatrix();
    
    glPopMatrix();
}
