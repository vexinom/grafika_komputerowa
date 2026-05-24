#pragma once

#include <vector> 
#include <glm/glm.hpp>

class Chunk
{
    public:

    unsigned VAO, VBO;
    unsigned EBO[3]; //for 3 level of details high/medium/low far away chunks are drawn in lower quality
    int indexCount[3];

    int x, z;

    glm::vec3 minBoundBox;
    glm::vec3 maxBoundBox;

};

class Plane
{
public:
    glm::vec3 normal;
    float distance;

    Plane();
    Plane(const glm::vec4& vec);
    bool IsInFront(const glm::vec3 & point);
};

class WorldMesh
{
    public: 
        void Draw(glm::mat4 & viewProjection, glm::vec3 & cameraPosition);
        void Init();

        std::vector<Chunk> chunks;
        int indexCount;

        int width, height;
        unsigned int NUM_STRIPS;
        unsigned int NUM_VERTS_PER_STRIP;

        std::vector<float> vertices;
        std::vector<unsigned int> indices;
};

std::vector<Plane> GetFrustumPlanes(const glm::mat4 & viewProj);
bool IsBoxInFrustrum(const glm::vec3 & min, const glm::vec3 max, const std::vector<Plane> planes);