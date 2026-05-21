#include "worldmesh.h"
#include <vector>
#include <cmath>
#include <glad/glad.h>


float terrain_z(float x, float y)
{
    return 1.5f * (1.0f/ (1.0f + exp(x))) + 2.0f * (1.0f/ (0.5f + exp(y)));
}

void WorldMesh::CreatePlain(int N)
{
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    
    for (int x = 0; x < N; x++)
    {
        for (int y = 0; y < N; y++)
        {
            float fx = (float)x / (N - 1);
            float fy = (float)y / (N - 1);

            float worldX = fx * 10.0f;
            float worldZ = fy * 10.0f - 10.0f;

            float height = terrain_z(worldX, worldZ);

            vertices.push_back(worldX);
            vertices.push_back(height);
            vertices.push_back(worldZ);
        }
    }

     for (int x = 0; x < N - 1; x++)
    {
        for (int y = 0; y < N - 1; y++)
        {
            int i0 = x * N + y;
            int i1 = (x + 1) * N + y;
            int i2 = x * N + (y + 1);
            int i3 = (x + 1) * N + (y + 1);

            indices.push_back(i0);
            indices.push_back(i1);
            indices.push_back(i2);

            indices.push_back(i2);
            indices.push_back(i1);
            indices.push_back(i3);
        }
    }

    indexCount = indices.size();

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);


    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void WorldMesh::Draw()
{
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
}


