#include "monument.h"
#include <vector>
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

#include "objloader.h"

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
        out.push_back(0.0f); out.push_back(0.0f);
    }
}

void Monument::Init(float worldX, float worldZ, float baseY, float topY)
{
    std::vector<float> vertices;

    ObjMesh lighthouse;
    bool ok = LoadObj("assets/lighthouse/source/Lighthouse_Low.obj", lighthouse);

    if (ok)
    {
        for (size_t i = 0; i < lighthouse.indices.size(); ++i)
        {
            unsigned int idx = lighthouse.indices[i];
            size_t base = static_cast<size_t>(idx) * 8;

            vertices.push_back(lighthouse.interleaved[base + 0]);
            vertices.push_back(lighthouse.interleaved[base + 1]);
            vertices.push_back(lighthouse.interleaved[base + 2]);
            vertices.push_back(lighthouse.interleaved[base + 3]);
            vertices.push_back(lighthouse.interleaved[base + 4]);
            vertices.push_back(lighthouse.interleaved[base + 5]);
            vertices.push_back(lighthouse.interleaved[base + 6]);
            vertices.push_back(lighthouse.interleaved[base + 7]);
        }

        const float targetHeight = 178.0f;
        const float terrainLift = 1.0f;
        const float sizeScale = targetHeight * lighthouse.invExtent;

        // Anchor point for the shadow frustum: middle of the lighthouse body.
        worldHeight = targetHeight;
        worldCenter = glm::vec3(worldX, baseY + terrainLift + targetHeight * 0.5f, worldZ);

        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(worldX, baseY + terrainLift, worldZ));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(sizeScale));
        modelMatrix = glm::translate(modelMatrix, glm::vec3(-lighthouse.center.x, -lighthouse.minY, -lighthouse.center.z));

        albedoTex = LoadTexture("assets/lighthouse/textures/Lighthouse_Low_blinn1SG_BaseColor.png", true);
        useAlbedoTex = true;
    }
    else
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

        for (int k = 0; k < 4; k++)
        {
            int n = (k + 1) % 4;
            PushTriangle(vertices, ring0[k], ring0[n], ring1[n], axis);
            PushTriangle(vertices, ring0[k], ring1[n], ring1[k], axis);
            PushTriangle(vertices, ring1[k], ring1[n], apex, axis);
        }

        modelMatrix = glm::mat4(1.0f);
        useAlbedoTex = false;

        worldHeight = (topY + capHeight) - baseY;
        worldCenter = glm::vec3(worldX, baseY + worldHeight * 0.5f, worldZ);
    }

    vertexCount = (int)(vertices.size() / 8);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
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
    shader.SetMat4("model", modelMatrix);
    shader.SetMat4("lightSpaceMatrix", lightSpaceMatrix);
    shader.SetInt("albedoMap", 0);
    shader.SetInt("useAlbedoMap", useAlbedoTex ? 1 : 0);
    shader.SetInt("shadowMap", 1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, useAlbedoTex ? albedoTex : 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, shadowMap);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    glBindVertexArray(0);
}

void Monument::DrawDepth(Shader& shader, const glm::mat4& lightSpaceMatrix)
{
    shader.SetMat4("lightSpaceMatrix", lightSpaceMatrix);
    shader.SetMat4("model", modelMatrix);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    glBindVertexArray(0);
}
