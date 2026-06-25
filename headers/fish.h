#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "shader.h"

class Fish {
public:
    void Init();
    void Update(float dt, const glm::vec3& cameraPos);
    void Draw(Shader& shader, const glm::mat4& view, const glm::mat4& projection,
              const glm::vec3& sunDirection, const glm::vec3& cameraPos);

    glm::vec3 Position() const { return schoolCenters.empty() ? glm::vec3(0.0f) : schoolCenters[0]; }

    void SetPredator(const glm::vec3& pos, float radius);
    int  FindNearestFish(const glm::vec3& from, float maxDist, glm::vec3& outPos) const;
    glm::vec3 FishPos(int index) const;
    bool FishAlive(int index) const;
    void EatFish(int index);

private:
    struct Boid { glm::vec3 pos; glm::vec3 vel; };

    float SeabedHeight(float x, float z) const;

    std::vector<Boid>      boids;
    std::vector<glm::mat4> models;
    std::vector<glm::vec3> schoolCenters;

    unsigned int VAO=0, VBO=0, EBO=0, instanceVBO=0;
    int indexCount=0;
    unsigned int albedoTex=0;

    glm::vec3 modelCenter = glm::vec3(0.0f);
    float     fishScale   = 1.0f;
    float     animTime    = 0.0f;

    unsigned short* heightData = nullptr;
    int hmW = 0, hmH = 0;

    float schoolRadius = 150.0f;

    std::vector<char>  eaten;
    std::vector<float> respawn;
    glm::vec3 predatorPos    = glm::vec3(1e9f);
    float     predatorRadius = 0.0f;
    bool      predatorActive = false;
};
