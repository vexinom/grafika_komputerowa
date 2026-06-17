#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "shader.h"

// CPU-updated particle system: rising air bubbles (B01) + billboarded marine
// snow / plankton drifting in the current (B02). Particles wrap inside a box
// that follows the camera so the volume always feels populated.
class Particles
{
    public:
        void Init(int count = 1400);
        void Update(float dt, const glm::vec3& cameraPos, float time, const glm::vec3& current);
        void Draw(Shader& shader, const glm::mat4& view, const glm::mat4& projection, float viewportHeight);

    private:
        struct P
        {
            glm::vec3 pos;
            float size;
            float speed;
            float phase;
            float type;   // 0 = marine snow, 1 = bubble
        };

        std::vector<P> particles;
        std::vector<float> buffer;       // interleaved pos(3) size(1) type(1)
        unsigned int VAO = 0, VBO = 0;
        glm::vec3 half = glm::vec3(230.0f, 70.0f, 230.0f);
        float yMin = -235.0f;            // fill the whole water column, from the deep floor...
        float yMax = 77.0f;              // ...up to just below the surface (water at ~80)
};
