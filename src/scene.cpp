#include "scene.h"
#include "skydome.h"


void Scene::Init()
{
    worldmesh.Init();
    watermesh.Init(90.0f, 50.0f);
    skydome.Init();
}


void Scene::DailyCycle(float time)
{

    float timeSpeed = time * 0.02f;
    sun.direction.x = cos(timeSpeed);
    sun.direction.y = sin(timeSpeed);
    sun.direction.z = -0.5f;
    sun.direction = glm::normalize(sun.direction);

    //unsigned int sunDirLoc = glGetUniformLocation(skydomeShaderID, "sunDirection");
    //glUniform3fv(sunDirLoc, 1, &sun.direction[0]);
}