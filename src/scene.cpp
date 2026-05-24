#include "scene.h"

void Scene::Init()
{

    terrain.Init();
}

void Scene::Render(glm::mat4 viewProjection, glm::vec3 cameraPosition)
{
    terrain.Draw(viewProjection, cameraPosition);
}