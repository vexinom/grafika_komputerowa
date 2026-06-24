#pragma once

#include <vector> 
#include <glm/glm.hpp>
#include "shader.h"
#include "config.h"

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
        void Draw(Shader& shader, const glm::mat4 & view, glm::mat4 & projection, const glm::vec3& cameraPosition, 
                const glm::vec3& sunDirection, const glm::mat4& lightSpaceMatrix, unsigned int shadowMap, glm::vec4 & clipPlane);
        void DrawDepth(Shader& shader, const glm::mat4& lightSpaceMatrix, const glm::vec3& cameraPosition);
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
        unsigned int surfaceNormalTexture;
        unsigned int grassNormalTexture;

        int width, height;
        float yScale = config::Y_SCALE_TERRAIN;     // flat, spacious, fully-submerged ocean floor
        float yShift = config::Y_SHIFT_TERRAIN;
        float skirtDepth = 5.0f;

        float metallic = 0.0f;
        float roughness = 0.88f;  // sand reads better as a rough, non-shiny surface

        glm::vec3 headlightPos = glm::vec3(0.0f);
        glm::vec3 headlightColor = glm::vec3(0.0f);
};

std::vector<Plane> GetFrustumPlanes(const glm::mat4 & viewProj);
bool IsBoxInFrustrum(const glm::vec3 & min, const glm::vec3 max, const std::vector<Plane>& planes);
glm::vec3 GetVertexNormal(int globalX, int globalZ, float * data, int width, int height, int channels, float yScale);