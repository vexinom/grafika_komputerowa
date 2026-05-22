#pragma once

#include <vector> 

class WorldMesh
{
    public: 
        void Draw();
        void Init();

        unsigned int VAO, VBO, EBO;
        int indexCount;

        int width, height;
        unsigned int NUM_STRIPS;
        unsigned int NUM_VERTS_PER_STRIP;

        std::vector<float> vertices;
        std::vector<unsigned int> indices;
};