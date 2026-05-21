#pragma once

class WorldMesh
{
    public: 
        void CreatePlain(int N);
        void Draw();

    private:
        unsigned int VAO, VBO, EBO;
        int indexCount;
};