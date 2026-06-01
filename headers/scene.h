#pragma once
#include "camera.h"
#include "worldmesh.h"
#include "watermesh.h"
#include "skydome.h"
#include "shader.h"

class Scene
{
    public:
        Camera camera;
        WorldMesh terrain;
        WaterMesh water;
        Skydome skydome;

        Scene()
        {
            
        }

        void Init();
        void Update();
        void Render(glm::mat4 viewProjection, glm::vec3 cameraPosition, unsigned int shaderID); 
};

