#include "scene.h"
#include "skydome.h"


void Scene::Init()
{
    worldmesh.Init();
    watermesh.Init();
    skydome.Init();
    cubemap.Init();
    tube.Init();
    monument.Init(1320.0f, 1700.0f, -10.0f, 50.0f);   // ruined pillar on the shelf
    axolotl.Init();
    fish.Init();
    otter.Init();
    reef.Init();
    islandPalms.Init();
    particles.Init(2400);
}


void Scene::DailyCycle(float time)
{

    float timeSpeed = time * 0.02f;
    sun.direction.x = cos(timeSpeed);
    sun.direction.y = sin(timeSpeed);
    sun.direction.z = -0.5f;
    sun.direction = glm::normalize(sun.direction);

    
}