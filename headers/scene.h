#pragma once
#include "camera.h"
#include "worldmesh.h"
#include "watermesh.h"
#include "skydome.h"
#include "cubemap.h"
#include "tube.h"
#include "monument.h"
#include "axolotl.h"
#include "fish.h"
#include "otter.h"
#include "reef.h"
#include "seaweed.h"
#include "islandPalms.h"
#include "particles.h"
#include "seagull.h"
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
        WorldMesh worldmesh;
        WaterMesh watermesh;
        Skydome skydome;
        Cubemap cubemap;
        Tube tube;
        Monument monument;
        Axolotl axolotl;
        Fish fish;
        Otter otter;
        Reef reef;
        Seaweed seaweed;
        IslandPalms islandPalms;
        Particles particles;
        Seagull seagull;
        Sun sun;

        // user-controllable environment state (interactions)
        bool  headlightOn = true;
        float fogDensity = 0.6f;             // underwater visibility multiplier (lower = clearer)
        glm::vec3 current = glm::vec3(8.0f, 0.0f, 4.0f);   // water-current drift

        void Init();
        void DailyCycle(float time);
};
