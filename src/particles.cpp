#include "particles.h"
#include <glad/glad.h>
#include <cstdlib>
#include <cmath>

static float frand() { return (float)rand() / (float)RAND_MAX; }
static float frand(float a, float b) { return a + (b - a) * frand(); }

// wrap v into [-h, h]
static float wrap(float v, float h)
{
    float span = 2.0f * h;
    v = fmod(v + h, span);
    if (v < 0.0f) v += span;
    return v - h;
}

void Particles::Init(int count)
{
    srand(1337);
    particles.resize(count);
    for (int i = 0; i < count; i++)
    {
        P& p = particles[i];
        p.type  = (frand() < 0.30f) ? 1.0f : 0.0f;   // ~30% bubbles
        p.pos   = glm::vec3(frand(-half.x, half.x), frand(-half.y, half.y), frand(-half.z, half.z));
        p.phase = frand(0.0f, 6.2831f);
        if (p.type > 0.5f) { p.size = frand(0.7f, 2.2f); p.speed = frand(14.0f, 30.0f); }
        else               { p.size = frand(0.3f, 0.9f); p.speed = frand(2.0f, 6.0f); }
    }
    buffer.resize(count * 5);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(4 * sizeof(float)));
    glBindVertexArray(0);
}

void Particles::Update(float dt, const glm::vec3& cameraPos, float time, const glm::vec3& current)
{
    if (dt > 0.05f) dt = 0.05f;

    for (size_t i = 0; i < particles.size(); i++)
    {
        P& p = particles[i];

        if (p.type > 0.5f)
        {
            // bubbles rise and wobble
            p.pos.y += p.speed * dt;
            p.pos.x += sinf(time * 2.0f + p.phase) * 6.0f * dt + current.x * dt;
            p.pos.z += cosf(time * 1.7f + p.phase) * 6.0f * dt + current.z * dt;
        }
        else
        {
            // marine snow drifts slowly with the current
            p.pos.y -= p.speed * 0.25f * dt;
            p.pos.x += sinf(time * 0.4f + p.phase) * 3.0f * dt + current.x * dt;
            p.pos.z += cosf(time * 0.5f + p.phase) * 3.0f * dt + current.z * dt;
        }

        // follow the camera horizontally, but keep the vertical extent in a fixed
        // band that stays strictly below the water surface (no snow in the sky)
        float rx = wrap(p.pos.x - cameraPos.x, half.x);
        float rz = wrap(p.pos.z - cameraPos.z, half.z);
        p.pos.x = cameraPos.x + rx;
        p.pos.z = cameraPos.z + rz;

        float span = yMax - yMin;
        float yy = fmod(p.pos.y - yMin, span);
        if (yy < 0.0f) yy += span;
        p.pos.y = yMin + yy;

        buffer[i * 5 + 0] = p.pos.x;
        buffer[i * 5 + 1] = p.pos.y;
        buffer[i * 5 + 2] = p.pos.z;
        buffer[i * 5 + 3] = p.size;
        buffer[i * 5 + 4] = p.type;
    }

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, buffer.size() * sizeof(float), buffer.data());
}

void Particles::Draw(Shader& shader, const glm::mat4& view, const glm::mat4& projection, float viewportHeight)
{
    shader.Use();
    shader.SetMat4("view", view);
    shader.SetMat4("projection", projection);
    // pixels = worldDiameter * (viewportHeight/2) * projection[1][1] / clipW
    shader.SetFloat("sizeScale", 0.5f * viewportHeight * projection[1][1]);

    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, (GLsizei)particles.size());
    glBindVertexArray(0);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glDisable(GL_PROGRAM_POINT_SIZE);
}
