#include "scene.h"

void Scene::Init()
{
    terrain.CreatePlain(100);
}

void Scene::Render()
{
    terrain.Draw();
}