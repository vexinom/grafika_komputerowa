#pragma once
#include "camera.h"
#include "worldmesh.h"
#include "watermesh.h"

class Scene
{
    public:
        Camera camera;
        WorldMesh terrain;
        WaterMesh water;

        Scene()
        {
            
        }

        void Init();
        void Update();
        void Render(glm::mat4 viewProjection, glm::vec3 cameraPosition);

};