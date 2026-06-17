#include "scene.h"
#include "skydome.h"


void Scene::Init()
{
    worldmesh.Init();
    watermesh.Init(90.0f, 50.0f);
    skydome.Init();
    cubemap.Init();
    tube.Init();
    monument.Init(1320.0f, 1700.0f, -10.0f, 50.0f);   // ruined pillar on the shelf
    axolotl.Init();
    reef.Init();
    particles.Init(2400);
}


void Scene::DailyCycle(float time)
{
    // Fixed, high daytime sun. The previous day/night cycle frequently left the
    // scene dark (and disabled the god rays). A steady bright sun keeps the
    // underwater scene readable, shadows crisp and the light shafts always on.
    (void)time;
    sun.direction = glm::normalize(glm::vec3(0.35f, 0.82f, -0.45f));
}