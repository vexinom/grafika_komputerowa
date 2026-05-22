#include "scene.h"

void Scene::Init()
{

    terrain.Init();
}

void Scene::Render()
{
    terrain.Draw();
}