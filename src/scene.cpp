#include "scene.h"
#include "skydome.h"


void Scene::Init()
{
    terrain.Init();
    water.Init(90.0f, 50.0f);
    skydome.Init();
}

void Scene::Render(glm::mat4 viewProjection, glm::vec3 cameraPosition, unsigned int shaderID)
{
    terrain.Draw(viewProjection, cameraPosition, shaderID);
}