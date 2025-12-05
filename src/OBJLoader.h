#ifndef OBJLOADER_H
#define OBJLOADER_H

#include <vector>
#include <string>
#include <GL/glut.h>

struct Vec3 {
    float x, y, z;
    Vec3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}
};

struct Face {
    int v[3];  // vertex indices
    int n[3];  // normal indices (if available)
};

struct MeshObject {
    std::string name;
    std::vector<Face> faces;
    int startVertex;
    Vec3 center;
    Vec3 min, max;
};

class OBJModel {
public:
    OBJModel();
    bool load(const std::string& filename);
    void draw(int objectIndex = -1);
    void clear();
    int getObjectCount() const { return objects.size(); }
    std::string getObjectName(int index) const;
    
private:
    std::vector<Vec3> vertices;
    std::vector<Vec3> normals;
    std::vector<MeshObject> objects;
    bool hasNormals;
};

#endif
