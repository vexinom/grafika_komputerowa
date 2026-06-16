#include "scene.h"
#include "skydome.h"


void Scene::Init()
{
    worldmesh.Init();
    watermesh.Init(90.0f, 50.0f);
    skydome.Init();
<<<<<<< HEAD
    cubemap.Init();
    tube.Init();
    monument.Init(1224.0f, 926.0f, 70.0f, 200.0f);
    axolotl.Init();
=======
>>>>>>> c03a168ac1ee55ce4304085a785f225cd8fd36c5
}


void Scene::DailyCycle(float time)
{

    float timeSpeed = time * 0.02f;
    sun.direction.x = cos(timeSpeed);
    sun.direction.y = sin(timeSpeed);
    sun.direction.z = -0.5f;
    sun.direction = glm::normalize(sun.direction);

    
}