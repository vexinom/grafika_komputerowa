#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "shader.h"

class AxolotlInstance
{
    public:
        std::vector<glm::vec3> pathPos;   // sampled spline positions
        std::vector<glm::vec3> pathTan;   // unit tangents
        std::vector<glm::vec3> pathNrm;   // parallel-transported (rotation-minimizing) normals
        std::vector<float> cumLen;
        float totalLen;
        float dist;
        float scale;
        float boost = 0.0f;               // temporary speed boost (set when poked)
        glm::vec3 currentPos;
        glm::vec3 forward;
        glm::mat4 model;
};

class Axolotl
{
    public:
        void Init();
        void Update(float dt);
        void Draw(Shader& shader, const glm::mat4& view, const glm::mat4& projection, const glm::vec3& sunDirection, const glm::vec3& cameraPos, const glm::mat4& lightSpaceMatrix, unsigned int shadowMap);
        void DrawDepth(Shader& shader, const glm::mat4& lightSpaceMatrix);

        glm::mat4 ModelMatrix(const glm::vec3& position, const glm::vec3& forward, float scale) const;
        void DrawSingle(Shader& shader, const glm::mat4& view, const glm::mat4& projection, const glm::vec3& sunDirection, const glm::vec3& cameraPos, const glm::mat4& lightSpaceMatrix, unsigned int shadowMap, const glm::mat4& model);
        void DrawSingleDepth(Shader& shader, const glm::mat4& lightSpaceMatrix, const glm::mat4& model);
        
        glm::vec3 NearestTo(const glm::vec3& point) const;
        glm::vec3 HeadlightPosition() const;

        // interaction hooks
        void Poke(const glm::vec3& from);  // ray-pick: speeds up the nearest creature
        bool  paused = false;              // freeze movement (keep swimming animation)
        float speedScale = 1.0f;           // user-controlled cruise speed multiplier

    private:
        unsigned int VAO, VBO, EBO;
        unsigned int baseColorTex, opacityTex, metallicTex, roughnessTex;
        int indexCount;

        glm::vec3 bboxCenter;
        float baseScale;
        float bodyMinY;
        float bodyLenY;
        float animTime;

        std::vector<AxolotlInstance> instances;
        void BindShaderState(Shader& shader, const glm::mat4& view, const glm::mat4& projection,
                         const glm::vec3& sunDirection, const glm::vec3& cameraPos,
                         const glm::mat4& lightSpaceMatrix, unsigned int shadowMap);
};
