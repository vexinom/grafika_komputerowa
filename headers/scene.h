#pragma once
#include "camera.h"
#include "worldmesh.h"
#include "watermesh.h"
#include "skydome.h"
#include "shader.h"

class Sun
{   public:
        glm::vec3 direction = glm::normalize(glm::vec3(0.5f, 0.4f, -1.0f));
        glm::vec3 color = glm::vec3(1.0f, 0.95f, 0.8f);

};


class Scene
{
    public:
        Camera camera;
        WorldMesh terrain;
        WaterMesh water;
        Skydome skydome;
        Sun sun;


        void Init();
        void DailyCycle(float time);
};

