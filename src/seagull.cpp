#include "seagull.h"

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <cstdio>

#include "objloader.h"

Seagull::SubMesh Seagull::LoadSub(const std::string& objPath, const std::string& texPath)
{
    SubMesh m;

    ObjMesh mesh;
    if (!LoadObj(objPath, mesh))
    {
        fprintf(stderr, "Seagull: nie udalo sie wczytac %s\n", objPath.c_str());
        return m;
    }

    glGenVertexArrays(1, &m.VAO);
    glGenBuffers(1, &m.VBO);
    glGenBuffers(1, &m.EBO);

    glBindVertexArray(m.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m.VBO);
    glBufferData(GL_ARRAY_BUFFER, mesh.interleaved.size() * sizeof(float), mesh.interleaved.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(unsigned int), mesh.indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    glBindVertexArray(0);

    m.indexCount = (int)mesh.indices.size();
    // glTF UVs use a top-left origin, so load the texture WITHOUT the legacy flip.
    m.tex = LoadTexture(texPath, false);
    return m;
}

void Seagull::Init(const glm::vec3& lighthouseCenter, float lighthouseHeight)
{
    wings = LoadSub("assets/seagull/seagull_wings.obj", "assets/seagull/textures/M_SeagullWings_diffuse.png");
    body  = LoadSub("assets/seagull/seagull_body.obj",  "assets/seagull/textures/M_SeagullBody_diffuse.png");

    axis = glm::vec3(lighthouseCenter.x, 0.0f, lighthouseCenter.z);
    // Circle around the upper part of the lighthouse (near the lantern).
    orbitBaseY = lighthouseCenter.y + lighthouseHeight * 0.5f - 30.0f;

    const float PI = 3.14159265359f;

    // A tight, synchronised PAIR plus a few looser birds for life. Tweak freely.
    birds = {
        //   radius baseAngle   omega   height   bob bobSpd   bank  scale flapSpd flapAmp  phase
        {  185.0f,   0.0f,     0.55f,   10.0f,   8.0f, 0.9f,  0.34f, 82.0f,  7.5f,  0.15f,  0.0f },  // pair A
        {  185.0f,    PI,      0.55f,   10.0f,   8.0f, 0.9f,  0.34f, 82.0f,  7.5f,  0.15f,  1.7f },  // pair B (opposite side)
        {  255.0f,  1.2f,      0.34f,   55.0f,  14.0f, 0.7f,  0.26f, 92.0f,  5.5f,  0.12f,  2.4f },
        {  320.0f,  2.7f,     -0.30f,   85.0f,  18.0f, 0.6f, -0.24f, 76.0f,  4.5f,  0.09f,  4.1f },
        {  240.0f,  4.6f,     -0.42f,  -25.0f,  10.0f, 0.8f, -0.30f, 70.0f,  6.5f,  0.13f,  0.9f },
        {  300.0f,  5.4f,      0.24f,  120.0f,  16.0f, 0.5f,  0.20f, 64.0f,  4.0f,  0.07f,  3.3f },
    };

    models.assign(birds.size(), glm::mat4(1.0f));
}

void Seagull::Update(float time)
{
    animTime = time;

    for (size_t i = 0; i < birds.size(); ++i)
    {
        const Bird& b = birds[i];

        float angle = b.baseAngle + b.omega * time;
        float ca = std::cos(angle);
        float sa = std::sin(angle);

        glm::vec3 pos = axis + b.radius * glm::vec3(ca, 0.0f, sa);
        pos.y = orbitBaseY + b.height + b.bob * std::sin(time * b.bobSpeed + b.phase);

        // Forward = travel direction (tangent to the circle, in the omega sign).
        float s = (b.omega >= 0.0f) ? 1.0f : -1.0f;
        glm::vec3 fwd = glm::normalize(glm::vec3(-sa, 0.0f, ca) * s);

        // Level basis, then roll INTO the turn (top leans toward the circle
        // centre) so the bird banks like a real one.
        glm::vec3 right0 = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), fwd));
        glm::vec3 up0    = glm::cross(fwd, right0);

        float roll = -b.bank * s;
        float cr = std::cos(roll);
        float sr = std::sin(roll);
        glm::vec3 up    = up0 * cr + right0 * sr;
        glm::vec3 right = right0 * cr - up0 * sr;

        // Columns map the model's local axes (x=wing, y=up, z=forward/beak) to world.
        glm::mat4 basis(1.0f);
        basis[0] = glm::vec4(right, 0.0f);
        basis[1] = glm::vec4(up,    0.0f);
        basis[2] = glm::vec4(fwd,   0.0f);

        glm::mat4 model = glm::translate(glm::mat4(1.0f), pos) * basis;
        model = glm::scale(model, glm::vec3(b.scale));
        models[i] = model;
    }
}

void Seagull::Draw(Shader& shader,
                   const glm::mat4& view,
                   const glm::mat4& projection,
                   const glm::vec3& sunDirection,
                   const glm::vec3& cameraPos,
                   const glm::mat4& lightSpaceMatrix,
                   unsigned int shadowMap)
{
    if (models.empty()) return;

    shader.Use();
    shader.SetMat4("view", view);
    shader.SetMat4("projection", projection);
    shader.SetVec3("sunDirection", sunDirection);
    shader.SetVec3("cameraPos", cameraPos);
    shader.SetMat4("lightSpaceMatrix", lightSpaceMatrix);
    shader.SetFloat("uTime", animTime);
    shader.SetInt("albedoMap", 0);
    shader.SetInt("shadowMap", 1);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, shadowMap);

    // Thin wings -> draw both faces so they are never culled away.
    GLboolean cullWasOn = glIsEnabled(GL_CULL_FACE);
    glDisable(GL_CULL_FACE);

    for (size_t i = 0; i < birds.size(); ++i)
    {
        const Bird& b = birds[i];
        shader.SetMat4("model", models[i]);
        shader.SetFloat("uPhase", b.phase);
        shader.SetFloat("uFlapSpeed", b.flapSpeed);
        shader.SetFloat("uFlapAmp", b.flapAmp);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, wings.tex);
        glBindVertexArray(wings.VAO);
        glDrawElements(GL_TRIANGLES, wings.indexCount, GL_UNSIGNED_INT, 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, body.tex);
        glBindVertexArray(body.VAO);
        glDrawElements(GL_TRIANGLES, body.indexCount, GL_UNSIGNED_INT, 0);
    }

    glBindVertexArray(0);
    if (cullWasOn) glEnable(GL_CULL_FACE);
}
