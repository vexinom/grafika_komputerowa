#include "seaweed.h"
#include <vector>
#include <cmath>
#include <cstdlib>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "stb_image.h"

static float fr() { return (float)rand() / (float)RAND_MAX; }
static float fr(float a, float b) { return a + (b - a) * fr(); }

void Seaweed::Init()
{
    stbi_set_flip_vertically_on_load(true);
    int w = 0, h = 0, n = 0;
    unsigned char* hm = stbi_load("assets/worldmap.png", &w, &h, &n, 1);
    const float DEEP = -700.0f, SHELF = 30.0f, ISLAND = 320.0f;   // must match worldmesh_vertex.glsl

    // radial seabed height -- must match worldmesh_vertex.glsl terrainHeight()
    auto seabed = [&](float x, float z) -> float
    {
        float fw = w ? (float)w : 2048.0f, fh = h ? (float)h : 2048.0f;
        float u = x / fw, v = z / fh;
        float s = 0.5f;
        if (hm) { int xi = (int)glm::clamp(x, 0.0f, fw - 1.0f); int zi = (int)glm::clamp(z, 0.0f, fh - 1.0f); s = hm[zi * w + xi] / 255.0f; }
        float rn = glm::length(glm::vec2(u - 0.5f, v - 0.5f)) * 2.0f;
        float slope = glm::smoothstep(0.40f, 0.72f, rn);
        float base = glm::mix(DEEP, SHELF, slope);
        float isl = glm::smoothstep(0.90f, 1.30f, rn);
        base = glm::mix(base, ISLAND, isl);
        float deepAmt = 1.0f - slope;
        float amt = 18.0f + 160.0f * deepAmt + 190.0f * isl;
        float hh = base + (s - 0.5f) * 2.0f * amt;
        hh -= isl * (1.0f - s) * 240.0f;
        return hh;
    };

    float mapW = (float)(w ? w : 2048), mapH = (float)(h ? h : 2048);

    // kelp patches on the shallow shelf
    srand(777);
    std::vector<glm::vec2> patches;
    {
        int a = 0;
        while ((int)patches.size() < 30 && a < 30 * 80)
        {
            a++;
            float cx = fr(120.0f, mapW - 120.0f), cz = fr(120.0f, mapH - 120.0f);
            float fy = seabed(cx, cz);
            if (fy < -5.0f || fy > 72.0f) continue;     // shallow sandy shelf only
            patches.push_back(glm::vec2(cx, cz));
        }
    }

    std::vector<float> verts;       // pos(3) + info(4: heightFrac, phase, swayX, swayZ)
    std::vector<unsigned int> idx;
    unsigned int base = 0;
    const int SEG = 7;
    const int target = 1300;
    int placed = 0, attempts = 0;

    while (placed < target && attempts < target * 40)
    {
        attempts++;
        float x, z;
        if (!patches.empty() && fr() < 0.9f)
        {
            glm::vec2 c = patches[rand() % patches.size()];
            float ang = fr(0.0f, 6.2831f), rad = fr(0.0f, 120.0f);
            x = c.x + cosf(ang) * rad; z = c.y + sinf(ang) * rad;
        }
        else { x = fr(80.0f, mapW - 80.0f); z = fr(80.0f, mapH - 80.0f); }
        if (x < 4 || z < 4 || x > mapW - 4 || z > mapH - 4) continue;

        float floorY = seabed(x, z);
        if (floorY < -8.0f || floorY > 74.0f) continue;   // shelf shallows
        placed++;

        float height = fr(45.0f, 120.0f);
        float width  = fr(6.0f, 12.0f);
        float a = fr(0.0f, 6.2831f);
        glm::vec2 wdir(cosf(a), sinf(a));      // ribbon width direction (XZ)
        glm::vec2 sway(-wdir.y, wdir.x);       // bends along its thin axis
        float phase = fr(0.0f, 6.2831f);

        for (int l = 0; l <= SEG; l++)
        {
            float t = (float)l / (float)SEG;
            float y = floorY + t * height;
            float hw = glm::mix(1.0f, 0.25f, t) * width * 0.5f;   // taper toward tip
            for (int side = -1; side <= 1; side += 2)
            {
                float px = x + wdir.x * hw * (float)side;
                float pz = z + wdir.y * hw * (float)side;
                verts.push_back(px); verts.push_back(y); verts.push_back(pz);
                verts.push_back(t); verts.push_back(phase); verts.push_back(sway.x); verts.push_back(sway.y);
            }
        }
        for (int l = 0; l < SEG; l++)
        {
            unsigned int a0 = base + l * 2, a1 = a0 + 1, b0 = a0 + 2, b1 = a0 + 3;
            idx.push_back(a0); idx.push_back(b0); idx.push_back(a1);
            idx.push_back(a1); idx.push_back(b0); idx.push_back(b1);
        }
        base += (SEG + 1) * 2;
    }

    if (hm) stbi_image_free(hm);
    indexCount = (int)idx.size();
    if (indexCount == 0) return;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(unsigned int), idx.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1); glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);
}

void Seaweed::Draw(Shader& shader, const glm::mat4& view, const glm::mat4& projection,
                   const glm::vec3& sunDirection, const glm::vec3& cameraPos,
                   float time, const glm::vec3& current)
{
    if (indexCount == 0) return;
    shader.Use();
    shader.SetMat4("view", view);
    shader.SetMat4("projection", projection);
    shader.SetVec3("sunDirection", sunDirection);
    shader.SetVec3("cameraPos", cameraPos);
    shader.SetFloat("time", time);
    shader.SetVec3("current", current);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
