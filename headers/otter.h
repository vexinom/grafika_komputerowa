#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "shader.h"

class Fish;

class Otter {
public:
    enum State { WANDER, HUNGRY, CHASE, ATTACK };

    void Init();
    void Update(float dt, const glm::vec3& cameraPos, Fish& fish);
    void Draw(Shader& shader, const glm::mat4& view, const glm::mat4& projection,
              const glm::vec3& sunDirection, const glm::vec3& cameraPos);

    glm::vec3 Position() const { return position; }
    float     Hunger() const { return hunger; }

    void CycleClip();
    void StopCycle();

private:
    struct Clip {
        std::string name;
        float duration = 0.0f;
        int   frameCount = 0;
        std::vector<float> frames;
    };
    struct Player { int clip = 0; float time = 0.0f; };

    int  FindClip(const std::string& key) const;
    void SetClip(int clip);
    void SampleClip(const Player& p, std::vector<float>& out) const;
    void EnterState(State s);
    void PickWanderTarget(bool wide);
    void PickWanderTargetAround(const glm::vec3& center);
    float frand(float a, float b);

    unsigned int VAO=0, VBO=0, EBO=0;
    int indexCount=0;
    unsigned int albedoTex=0;

    int jointCount=0;
    glm::vec3 modelCenter = glm::vec3(0.0f);
    float     extent      = 1.0f;
    std::vector<Clip> clips;

    Player cur, prev;
    float  blendRemaining = 0.0f;
    float  blendDuration  = 0.25f;
    std::vector<float> bufCur, bufPrev, jointUpload;

    State state = WANDER;
    int   wanderClip=0, fastClip=0, attackClip=0, idleClip=0, eatClip=0;
    float stateTimer = 0.0f;

    float hunger = 100.0f;
    int   preyIndex = -1;
    glm::vec3 preyPos = glm::vec3(0.0f);
    float attackTimer = 0.0f;

    // Attack sequence: phase 0 = lunge onto prey, phase 1 = eat in place.
    int       attackPhase = 0;
    float     eatTimer   = 0.0f;
    glm::vec3 strikePoint = glm::vec3(0.0f);
    glm::vec3 eatAnchor   = glm::vec3(0.0f);

    int   manualClip = -1;

    glm::vec3 position = glm::vec3(1200.0f, 160.0f, 1300.0f);
    glm::vec3 velocity = glm::vec3(0.0f);
    glm::vec3 wanderTarget = glm::vec3(1200.0f, 160.0f, 1300.0f);
    float yaw = 0.0f;
    glm::vec3 facing = glm::vec3(0.0f, 0.0f, -1.0f);
    float swimPhase = 0.0f;
    glm::mat4 model = glm::mat4(1.0f);
};
