#pragma once
#include "camera.h"
#include "worldmesh.h"

class Scene
{
    public:
        Camera camera;
        WorldMesh terrain;

        Scene()
        {
            
        }

        void Init();
        void Update();
        void Render();

};