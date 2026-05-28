#pragma once

#include <vector> 
#include <glm/glm.hpp>

class Chunk
{
    public:
    int x, z;

    glm::vec3 minBoundBox;
    glm::vec3 maxBoundBox;

};

struct LocalVertex
{
    float x, z;
    float isSkirt;
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
        void Draw(glm::mat4 & viewProjection, glm::vec3 & cameraPosition, unsigned int shaderID);
        void Init();

        ~WorldMesh();

        std::vector<Chunk> chunks;

        unsigned int globalVAO;
        unsigned int globalVBO;
        unsigned int globalEBO[3];
        int globalindexCount[3];

        unsigned int heightmapTexture;
        unsigned int surfaceTexture;
        unsigned int grassTexture;

        int width, height;
        float yScale = 146.0f;
        float yShift = 0.0f;
        float skirtDepth = 5.0f;
};

std::vector<Plane> GetFrustumPlanes(const glm::mat4 & viewProj);
bool IsBoxInFrustrum(const glm::vec3 & min, const glm::vec3 max, const std::vector<Plane> planes);
glm::vec3 GetVertexNormal(int globalX, int globalZ, float * data, int width, int height, int channels, float yScale);