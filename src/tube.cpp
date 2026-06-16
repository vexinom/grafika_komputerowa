#include "tube.h"
#include <vector>
#include <cmath>
#include <glad/glad.h>

static glm::vec3 CatmullRom(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, float t)
{
    float t2 = t * t;
    float t3 = t2 * t;
    return 0.5f * ((2.0f * p1) + (-p0 + p2) * t + (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2 + (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3);
}

static glm::vec3 RotateAroundAxis(const glm::vec3& v, const glm::vec3& axis, float angle)
{
    float c = cos(angle);
    float s = sin(angle);
    return v * c + glm::cross(axis, v) * s + axis * glm::dot(axis, v) * (1.0f - c);
}

void Tube::Init()
{
    std::vector<glm::vec3> controls = {
        glm::vec3(5.0f, 30.0f, -5.0f),
        glm::vec3(25.0f, 48.0f, -28.0f),
        glm::vec3(55.0f, 36.0f, -52.0f),
        glm::vec3(82.0f, 54.0f, -74.0f),
        glm::vec3(108.0f, 40.0f, -96.0f),
        glm::vec3(135.0f, 58.0f, -120.0f)
    };

    const int samplesPerSegment = 24;
    std::vector<glm::vec3> path;

    for (size_t i = 0; i + 1 < controls.size(); i++)
    {
        glm::vec3 p0 = controls[i == 0 ? 0 : i - 1];
        glm::vec3 p1 = controls[i];
        glm::vec3 p2 = controls[i + 1];
        glm::vec3 p3 = controls[i + 2 < controls.size() ? i + 2 : controls.size() - 1];

        for (int s = 0; s < samplesPerSegment; s++)
        {
            float t = (float)s / (float)samplesPerSegment;
            path.push_back(CatmullRom(p0, p1, p2, p3, t));
        }
    }
    path.push_back(controls.back());

    int count = (int)path.size();
    std::vector<glm::vec3> tangents(count);
    for (int i = 0; i < count; i++)
    {
        glm::vec3 a = path[i == 0 ? 0 : i - 1];
        glm::vec3 b = path[i + 1 < count ? i + 1 : count - 1];
        tangents[i] = glm::normalize(b - a);
    }

    std::vector<glm::vec3> normals(count);
    glm::vec3 up = glm::abs(tangents[0].y) > 0.99f ? glm::vec3(1.0f, 0.0f, 0.0f) : glm::vec3(0.0f, 1.0f, 0.0f);
    normals[0] = glm::normalize(up - tangents[0] * glm::dot(up, tangents[0]));

    for (int i = 1; i < count; i++)
    {
        glm::vec3 prevT = tangents[i - 1];
        glm::vec3 curT = tangents[i];
        glm::vec3 axis = glm::cross(prevT, curT);
        float len = glm::length(axis);

        if (len < 1e-6f)
        {
            normals[i] = normals[i - 1];
        }
        else
        {
            axis /= len;
            float angle = atan2(len, glm::dot(prevT, curT));
            normals[i] = RotateAroundAxis(normals[i - 1], axis, angle);
        }
        normals[i] = glm::normalize(normals[i] - curT * glm::dot(normals[i], curT));
    }

    const int radial = 16;
    const float radius = 3.0f;
    const float twoPi = 6.28318530718f;

    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    for (int i = 0; i < count; i++)
    {
        glm::vec3 N = normals[i];
        glm::vec3 B = glm::cross(tangents[i], N);
        for (int j = 0; j < radial; j++)
        {
            float theta = twoPi * (float)j / (float)radial;
            float ct = cos(theta);
            float st = sin(theta);
            glm::vec3 dir = ct * N + st * B;
            glm::vec3 pos = path[i] + radius * dir;

            vertices.push_back(pos.x); vertices.push_back(pos.y); vertices.push_back(pos.z);
            vertices.push_back(dir.x); vertices.push_back(dir.y); vertices.push_back(dir.z);
        }
    }

    for (int i = 0; i + 1 < count; i++)
    {
        for (int j = 0; j < radial; j++)
        {
            int a = i * radial + j;
            int b = i * radial + (j + 1) % radial;
            int c = (i + 1) * radial + j;
            int d = (i + 1) * radial + (j + 1) % radial;

            indices.push_back(a); indices.push_back(c); indices.push_back(b);
            indices.push_back(b); indices.push_back(c); indices.push_back(d);
        }
    }

    indexCount = (int)indices.size();

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindVertexArray(0);
}

void Tube::Draw(Shader& shader, const glm::mat4& view, const glm::mat4& projection, const glm::vec3& sunDirection, const glm::vec3& cameraPos)
{
    shader.Use();
    shader.SetMat4("view", view);
    shader.SetMat4("projection", projection);
    shader.SetVec3("sunDirection", sunDirection);
    shader.SetVec3("cameraPos", cameraPos);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
