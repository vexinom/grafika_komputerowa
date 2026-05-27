#include "scene.h"

void Scene::Init()
{
    terrain.Init();
    water.Init(terrain.width, terrain.height, 45.0f);
}

void Scene::Render(glm::mat4 viewProjection, glm::vec3 cameraPosition)
{
    terrain.Draw(viewProjection, cameraPosition);

    
}