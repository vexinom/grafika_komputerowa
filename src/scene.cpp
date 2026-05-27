#include "scene.h"

void Scene::Init()
{
    terrain.Init();
    water.Init(terrain.width, terrain.height, 90.0f);
}

void Scene::Render(glm::mat4 viewProjection, glm::vec3 cameraPosition, unsigned int shaderID)
{
    terrain.Draw(viewProjection, cameraPosition, shaderID);
}