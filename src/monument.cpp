#include "monument.h"
#include <vector>
#include <glad/glad.h>

static void PushTriangle(std::vector<float>& out, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec3& axis)
{
    glm::vec3 n = glm::normalize(glm::cross(b - a, c - a));
    glm::vec3 center = (a + b + c) / 3.0f;
    if (glm::dot(n, center - glm::vec3(axis.x, center.y, axis.z)) < 0.0f) n = -n;

    glm::vec3 p[3] = { a, b, c };
    for (int i = 0; i < 3; i++)
    {
        out.push_back(p[i].x); out.push_back(p[i].y); out.push_back(p[i].z);
        out.push_back(n.x); out.push_back(n.y); out.push_back(n.z);
    }
}

void Monument::Init(float worldX, float worldZ, float baseY, float topY)
{
    float baseHalf = 7.0f;
    float topHalf = 3.5f;
    float capHeight = 22.0f;
    glm::vec3 axis(worldX, 0.0f, worldZ);

    glm::vec3 ring0[4] = {
        glm::vec3(worldX - baseHalf, baseY, worldZ - baseHalf),
        glm::vec3(worldX + baseHalf, baseY, worldZ - baseHalf),
        glm::vec3(worldX + baseHalf, baseY, worldZ + baseHalf),
        glm::vec3(worldX - baseHalf, baseY, worldZ + baseHalf)
    };
    glm::vec3 ring1[4] = {
        glm::vec3(worldX - topHalf, topY, worldZ - topHalf),
        glm::vec3(worldX + topHalf, topY, worldZ - topHalf),
        glm::vec3(worldX + topHalf, topY, worldZ + topHalf),
        glm::vec3(worldX - topHalf, topY, worldZ + topHalf)
    };
    glm::vec3 apex(worldX, topY + capHeight, worldZ);

    std::vector<float> vertices;
    for (int k = 0; k < 4; k++)
    {
        int n = (k + 1) % 4;
        PushTriangle(vertices, ring0[k], ring0[n], ring1[n], axis);
        PushTriangle(vertices, ring0[k], ring1[n], ring1[k], axis);
        PushTriangle(vertices, ring1[k], ring1[n], apex, axis);
    }

    vertexCount = (int)(vertices.size() / 6);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);
}

void Monument::Draw(Shader& shader,
                    const glm::mat4& view,
                    const glm::mat4& projection,
                    const glm::vec3& sunDirection,
                    const glm::vec3& cameraPos,
                    const glm::mat4& lightSpaceMatrix,
                    unsigned int shadowMap)
{
    shader.Use();
    shader.SetMat4("view", view);
    shader.SetMat4("projection", projection);
    shader.SetVec3("sunDirection", sunDirection);
    shader.SetVec3("cameraPos", cameraPos);
    shader.SetMat4("lightSpaceMatrix", lightSpaceMatrix);
    shader.SetInt("shadowMap", 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, shadowMap);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    glBindVertexArray(0);
}

void Monument::DrawDepth(Shader& shader, const glm::mat4& lightSpaceMatrix)
{
    shader.SetMat4("lightSpaceMatrix", lightSpaceMatrix);
    shader.SetMat4("model", glm::mat4(1.0f));

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    glBindVertexArray(0);
}
