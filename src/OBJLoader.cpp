#include "OBJLoader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>

OBJModel::OBJModel() : hasNormals(false) {}

bool OBJModel::load(const std::string& filename) {
    clear();
    
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open OBJ file: " << filename << std::endl;
        return false;
    }
    
    MeshObject* currentObject = nullptr;
    
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string type;
        iss >> type;
        
        if (type == "o") {
            std::string objName;
            iss >> objName;
            
            MeshObject obj;
            obj.name = objName;
            obj.startVertex = vertices.size();
            objects.push_back(obj);
            currentObject = &objects.back();
        }
        else if (type == "v") {
            Vec3 v;
            iss >> v.x >> v.y >> v.z;
            vertices.push_back(v);
        }
        else if (type == "vn") {
            Vec3 n;
            iss >> n.x >> n.y >> n.z;
            normals.push_back(n);
            hasNormals = true;
        }
        else if (type == "f") {
            Face face;
            std::string vertex;
            int idx = 0;
            bool validFace = true;
            
            while (iss >> vertex && idx < 3) {
                try {
                    std::istringstream vss(vertex);
                    std::string v_str, vt_str, vn_str;
                    
                    std::getline(vss, v_str, '/');
                    int vertIdx = std::stoi(v_str) - 1;
                    
                    if (vertIdx < 0 || vertIdx >= (int)vertices.size()) {
                        validFace = false;
                        break;
                    }
                    
                    face.v[idx] = vertIdx;
                    
                    if (std::getline(vss, vt_str, '/')) {
                        if (std::getline(vss, vn_str, '/')) {
                            if (!vn_str.empty()) {
                                int normIdx = std::stoi(vn_str) - 1;
                                if (normIdx >= 0 && normIdx < (int)normals.size()) {
                                    face.n[idx] = normIdx;
                                }
                            }
                        }
                    } else if (vertex.find("//") != std::string::npos) {
                        size_t pos = vertex.find("//");
                        vn_str = vertex.substr(pos + 2);
                        if (!vn_str.empty()) {
                            int normIdx = std::stoi(vn_str) - 1;
                            if (normIdx >= 0 && normIdx < (int)normals.size()) {
                                face.n[idx] = normIdx;
                            }
                        }
                    }
                    
                    idx++;
                } catch (const std::exception& e) {
                    validFace = false;
                    break;
                }
            }
            
            if (idx == 3 && validFace && currentObject) {
                currentObject->faces.push_back(face);
            }
        }
    }
    
    file.close();
    
    if (objects.empty()) {
        MeshObject obj;
        obj.name = "default";
        obj.startVertex = 0;
        objects.push_back(obj);
    }
    
    return !vertices.empty();
}

void OBJModel::draw(int objectIndex) {
    if (objects.empty() || vertices.empty()) {
        return;
    }
    
    int startObj = (objectIndex >= 0 && objectIndex < (int)objects.size()) ? objectIndex : 0;
    int endObj = (objectIndex >= 0 && objectIndex < (int)objects.size()) ? objectIndex + 1 : objects.size();
    
    for (int objIdx = startObj; objIdx < endObj; objIdx++) {
        const MeshObject& obj = objects[objIdx];
        
        if (obj.faces.empty()) continue;
        
        glBegin(GL_TRIANGLES);
        
        for (size_t i = 0; i < obj.faces.size(); i++) {
            const Face& face = obj.faces[i];
            
            bool validFace = true;
            for (int j = 0; j < 3; j++) {
                if (face.v[j] < 0 || face.v[j] >= (int)vertices.size()) {
                    validFace = false;
                    break;
                }
            }
            
            if (!validFace) continue;
            
            for (int j = 0; j < 3; j++) {
                int vi = face.v[j];
                
                if (hasNormals && face.n[j] >= 0 && face.n[j] < (int)normals.size()) {
                    const Vec3& n = normals[face.n[j]];
                    glNormal3f(n.x, n.y, n.z);
                } else if (j == 0) {
                    const Vec3& v0 = vertices[face.v[0]];
                    const Vec3& v1 = vertices[face.v[1]];
                    const Vec3& v2 = vertices[face.v[2]];
                    
                    Vec3 edge1(v1.x - v0.x, v1.y - v0.y, v1.z - v0.z);
                    Vec3 edge2(v2.x - v0.x, v2.y - v0.y, v2.z - v0.z);
                    
                    Vec3 normal(
                        edge1.y * edge2.z - edge1.z * edge2.y,
                        edge1.z * edge2.x - edge1.x * edge2.z,
                        edge1.x * edge2.y - edge1.y * edge2.x
                    );
                    
                    float len = std::sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
                    if (len > 0.0001f) {
                        normal.x /= len;
                        normal.y /= len;
                        normal.z /= len;
                    }
                    
                    glNormal3f(normal.x, normal.y, normal.z);
                }
                
                const Vec3& v = vertices[vi];
                glVertex3f(v.x, v.y, v.z);
            }
        }
        
        glEnd();
    }
}

std::string OBJModel::getObjectName(int index) const {
    if (index >= 0 && index < (int)objects.size()) {
        return objects[index].name;
    }
    return "";
}

void OBJModel::clear() {
    vertices.clear();
    normals.clear();
    objects.clear();
    hasNormals = false;
}
