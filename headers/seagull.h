#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "shader.h"

// A small flock of seagulls that circle (and bank) around the lighthouse.
// The mesh comes from the skinned glTF asset, baked to its rest pose into two
// OBJ sub-meshes (wings + body) so it can be drawn with the normal pipeline.
// Wing-flap is faked procedurally in the vertex shader, so no skeletal data is
// needed for the birds to look alive while they fly.
class Seagull
{
    public:
        // lighthouseCenter / lighthouseHeight come from Monument so the flock is
        // anchored to wherever the lighthouse actually stands.
        void Init(const glm::vec3& lighthouseCenter, float lighthouseHeight);
        void Update(float time);
        void Draw(Shader& shader,
                  const glm::mat4& view,
                  const glm::mat4& projection,
                  const glm::vec3& sunDirection,
                  const glm::vec3& cameraPos,
                  const glm::mat4& lightSpaceMatrix,
                  unsigned int shadowMap);

    private:
        struct SubMesh
        {
            unsigned int VAO = 0;
            unsigned int VBO = 0;
            unsigned int EBO = 0;
            int indexCount = 0;
            unsigned int tex = 0;
        };

        struct Bird
        {
            float radius;     // orbit radius around the lighthouse axis
            float baseAngle;  // starting angle on the circle
            float omega;      // angular speed (sign = direction of travel)
            float height;     // vertical offset above the orbit base height
            float bob;        // vertical bobbing amplitude
            float bobSpeed;   // bobbing speed
            float bank;       // roll angle (lean into the turn)
            float scale;      // wingspan in world units
            float flapSpeed;  // wing-flap speed
            float flapAmp;    // wing-flap amplitude (local units)
            float phase;      // per-bird animation phase
        };

        SubMesh LoadSub(const std::string& objPath, const std::string& texPath);

        SubMesh wings;
        SubMesh body;
        std::vector<Bird> birds;
        std::vector<glm::mat4> models;

        glm::vec3 axis = glm::vec3(0.0f); // lighthouse position (orbit centre)
        float orbitBaseY = 0.0f;          // height the flock circles around
        float animTime = 0.0f;            // current time, forwarded to the shader
};
