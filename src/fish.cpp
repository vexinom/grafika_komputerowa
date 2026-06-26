#include "fish.h"
#include "objloader.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include "stb_image.h"
#include <cstdlib>
#include <cmath>

static float frand() { return (float)rand() / (float)RAND_MAX; }
static float frand(float a, float b) { return a + (b - a) * frand(); }
static glm::vec3 randUnit()
{
    float z = frand(-1.0f, 1.0f);
    float t = frand(0.0f, 6.2831853f);
    float r = sqrtf(glm::max(0.0f, 1.0f - z * z));
    return glm::vec3(r * cosf(t), r * sinf(t), z);
}
static glm::vec3 limit(const glm::vec3& v, float maxLen)
{
    float l = glm::length(v);
    return (l > maxLen && l > 1e-5f) ? v * (maxLen / l) : v;
}

static const int   SCHOOLS    = 4;
static const int   PER_SCHOOL = 60;
static const float WATER_LEVEL = 400.0f;
static const float Y_SCALE     = 500.0f;

// Hard on-map bounds (same box the otter swims in) so a fish can never leave the map.
static const float FISH_MINX = 650.0f,  FISH_MAXX = 3550.0f;
static const float FISH_MINZ = 450.0f,  FISH_MAXZ = 2950.0f;

float Fish::SeabedHeight(float x, float z) const
{
    float fw = hmW ? (float)hmW : 4096.0f, fh = hmH ? (float)hmH : 4096.0f;
    float s = 0.0f;
    if (heightData)
    {
        int xi = (int)glm::clamp(x, 0.0f, fw - 1.0f);
        int zi = (int)glm::clamp(z, 0.0f, fh - 1.0f);
        s = heightData[zi * hmW + xi] / 65535.0f;
    }
    return s * Y_SCALE;
}

void Fish::Init()
{
    ObjMesh m;
    if (!LoadObj("assets/models/fish/obj/fish.obj", m))
        return;
    indexCount  = (int)m.indices.size();
    modelCenter = m.center;
    fishScale   = 40.0f * m.invExtent;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &instanceVBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, m.interleaved.size() * sizeof(float), m.interleaved.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m.indices.size() * sizeof(unsigned int), m.indices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1); glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2); glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    const int total = SCHOOLS * PER_SCHOOL;
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, total * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);
    for (int i = 0; i < 4; i++)
    {
        glEnableVertexAttribArray(3 + i);
        glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(i * sizeof(glm::vec4)));
        glVertexAttribDivisor(3 + i, 1);
    }
    glBindVertexArray(0);

    albedoTex = LoadTexture("assets/models/fish/Texture/fish.png");

    int n = 0;
    stbi_set_flip_vertically_on_load(true);
    heightData = stbi_load_16("assets/worldmap.png", &hmW, &hmH, &n, 1);

    glm::vec2 spots[SCHOOLS] = {
        glm::vec2(1260.0f, 1100.0f),
        glm::vec2(2020.0f,  540.0f),
        glm::vec2(3380.0f, 1520.0f),
        glm::vec2(2280.0f, 2880.0f),
    };
    schoolCenters.clear();
    for (int s = 0; s < SCHOOLS; s++)
    {
        float fy = SeabedHeight(spots[s].x, spots[s].y);
        float cy = glm::clamp(fy + 45.0f, fy + 20.0f, WATER_LEVEL - 30.0f);
        schoolCenters.push_back(glm::vec3(spots[s].x, cy, spots[s].y));
    }

    srand(2024);
    boids.resize(total);
    models.resize(total);
    eaten.assign(total, 0);
    respawn.assign(total, 0.0f);
    for (int s = 0; s < SCHOOLS; s++)
        for (int k = 0; k < PER_SCHOOL; k++)
        {
            Boid& b = boids[s * PER_SCHOOL + k];
            b.pos = schoolCenters[s] + randUnit() * frand(0.0f, schoolRadius);
            b.vel = randUnit() * frand(18.0f, 30.0f);
        }
}

void Fish::Update(float dt, const glm::vec3& cameraPos)
{
    if (boids.empty()) return;
    if (dt > 0.05f) dt = 0.05f;
    animTime += dt;

    for (size_t i = 0; i < boids.size(); i++)
    {
        if (eaten[i])
        {
            respawn[i] -= dt;
            if (respawn[i] <= 0.0f)
            {
                int s = (int)i / PER_SCHOOL;
                eaten[i] = 0;
                boids[i].pos = schoolCenters[s] + randUnit() * frand(0.0f, schoolRadius);
                boids[i].vel = randUnit() * frand(18.0f, 30.0f);
            }
        }
    }

    const float perceptionR = 130.0f;
    const float sepR        = 60.0f;
    const float maxSpeed    = 50.0f;
    const float minSpeed    = 22.0f;
    const float maxForce    = 65.0f;
    const float avoidRadius = 120.0f;

    for (int s = 0; s < (int)schoolCenters.size(); s++)
    {
        int base = s * PER_SCHOOL;
        glm::vec3 center = schoolCenters[s];

        for (int i = base; i < base + PER_SCHOOL; i++)
        {
            if (eaten[i]) continue;
            glm::vec3 pi = boids[i].pos;
            glm::vec3 vi = boids[i].vel;

            glm::vec3 sep(0.0f), ali(0.0f), coh(0.0f);
            int na = 0, ns = 0;

            for (int j = base; j < base + PER_SCHOOL; j++)
            {
                if (i == j || eaten[j]) continue;
                glm::vec3 d = pi - boids[j].pos;
                float dist = glm::length(d);
                if (dist < perceptionR && dist > 1e-4f)
                {
                    ali += boids[j].vel;
                    coh += boids[j].pos;
                    na++;
                    if (dist < sepR) { sep += (d / dist) / dist; ns++; }
                }
            }

            glm::vec3 acc(0.0f);
            if (ns > 0)
            {
                sep /= (float)ns;
                // normalize(0) is NaN; guard against a zero-sum neighbourhood
                glm::vec3 sd = (glm::length(sep) > 1e-4f) ? glm::normalize(sep) : glm::vec3(0.0f);
                acc += limit(sd * maxSpeed - vi, maxForce) * 1.7f;
            }
            if (na > 0)
            {
                ali /= (float)na;
                glm::vec3 ad = (glm::length(ali) > 1e-4f) ? glm::normalize(ali) : glm::vec3(0.0f);
                acc += limit(ad * maxSpeed - vi, maxForce) * 1.0f;
                coh = coh / (float)na - pi;
                if (glm::length(coh) > 1e-4f)
                    acc += limit(glm::normalize(coh) * maxSpeed - vi, maxForce) * 0.9f;
            }

            glm::vec3 toC = center - pi;
            float dc = glm::length(toC);
            if (dc > schoolRadius && dc > 1e-4f)
                acc += limit(glm::normalize(toC) * maxSpeed - vi, maxForce) * 1.5f;

            glm::vec3 away = pi - cameraPos;
            float cd = glm::length(away);
            if (cd < avoidRadius && cd > 1e-4f)
                acc += (away / cd) * maxForce * (3.0f + 6.0f * (1.0f - cd / avoidRadius));

            if (predatorActive)
            {
                glm::vec3 awayP = pi - predatorPos;
                float pd = glm::length(awayP);
                if (pd < predatorRadius && pd > 1e-4f)
                    acc += (awayP / pd) * maxForce * (3.5f + 7.0f * (1.0f - pd / predatorRadius));
            }

            float floorY = SeabedHeight(pi.x, pi.z);
            float lowY   = floorY + 18.0f;
            float highY  = floorY + 130.0f;
            if (highY > WATER_LEVEL - 10.0f) highY = WATER_LEVEL - 10.0f;
            if (highY < lowY + 8.0f)         highY = lowY + 8.0f;
            if (pi.y < lowY)  acc += glm::vec3(0,  1, 0) * maxForce * 2.5f;
            if (pi.y > highY) acc += glm::vec3(0, -1, 0) * maxForce * 2.5f;

            vi += acc * dt;
            float sp = glm::length(vi);
            if (sp > maxSpeed)                     vi *= maxSpeed / sp;
            else if (sp < minSpeed && sp > 1e-4f)  vi *= minSpeed / sp;

            glm::vec3 np = pi + vi * dt;

            // The camera and predator only steer the fish through the speed-capped
            // soft forces above. No hard position snap onto a shell -- that teleport
            // was what dragged fish across the map while the otter/camera moved.

            float fY = SeabedHeight(np.x, np.z);
            float hardLow  = fY + 8.0f;
            float hardHigh = fY + 160.0f;
            if (hardHigh > WATER_LEVEL - 5.0f) hardHigh = WATER_LEVEL - 5.0f;
            if (hardHigh < hardLow + 5.0f)     hardHigh = hardLow + 5.0f;
            if (np.y < hardLow)  { np.y = hardLow;  if (vi.y < 0) vi.y = 0; }
            if (np.y > hardHigh) { np.y = hardHigh; if (vi.y > 0) vi.y = 0; }

            np.x = glm::clamp(np.x, FISH_MINX, FISH_MAXX);
            np.z = glm::clamp(np.z, FISH_MINZ, FISH_MAXZ);

            // safety net: respawn rather than vanish if a position ever goes non-finite
            if (!(std::isfinite(np.x) && std::isfinite(np.y) && std::isfinite(np.z)))
            {
                np = center + randUnit() * frand(0.0f, schoolRadius);
                vi = randUnit() * frand(18.0f, 30.0f);
            }

            boids[i].vel = vi;
            boids[i].pos = np;
        }
    }

    for (size_t i = 0; i < boids.size(); i++)
    {
        if (eaten[i]) { models[i] = glm::scale(glm::mat4(1.0f), glm::vec3(0.0f)); continue; }
        glm::vec3 f = (glm::length(boids[i].vel) > 1e-4f)
                          ? glm::normalize(boids[i].vel)
                          : glm::vec3(0.0f, 0.0f, 1.0f);
        glm::vec3 right = glm::cross(glm::vec3(0, 1, 0), f);
        float rl = glm::length(right);
        right = (rl > 1e-4f) ? right / rl : glm::vec3(1, 0, 0);
        glm::vec3 up = glm::cross(f, right);

        glm::mat4 rot(1.0f);
        rot[0] = glm::vec4(right, 0.0f);
        rot[1] = glm::vec4(up,    0.0f);
        rot[2] = glm::vec4(f,     0.0f);

        glm::mat4 M = glm::translate(glm::mat4(1.0f), boids[i].pos) * rot;
        M = glm::scale(M, glm::vec3(fishScale));
        M = glm::translate(M, -modelCenter);
        models[i] = M;
    }

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, models.size() * sizeof(glm::mat4), models.data());
}

void Fish::Draw(Shader& shader, const glm::mat4& view, const glm::mat4& projection,
                const glm::vec3& sunDirection, const glm::vec3& cameraPos)
{
    if (indexCount == 0 || boids.empty()) return;

    shader.Use();
    shader.SetMat4("view", view);
    shader.SetMat4("projection", projection);
    shader.SetVec3("sunDirection", sunDirection);
    shader.SetVec3("tint", glm::vec3(1.0f));
    shader.SetFloat("time", animTime);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, albedoTex);
    shader.SetInt("albedo", 0);

    glBindVertexArray(VAO);
    glDrawElementsInstanced(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0, (GLsizei)boids.size());
    glBindVertexArray(0);
}

void Fish::SetPredator(const glm::vec3& pos, float radius)
{
    predatorPos = pos;
    predatorRadius = radius;
    predatorActive = true;
}

glm::vec3 Fish::FishPos(int i) const
{
    return (i >= 0 && i < (int)boids.size()) ? boids[i].pos : glm::vec3(0.0f);
}

bool Fish::FishAlive(int i) const
{
    return i >= 0 && i < (int)boids.size() && !eaten[i];
}

void Fish::EatFish(int i)
{
    if (i >= 0 && i < (int)boids.size() && !eaten[i]) { eaten[i] = 1; respawn[i] = 12.0f; }
}

int Fish::FindNearestFish(const glm::vec3& from, float maxDist, glm::vec3& outPos) const
{
    int best = -1;
    float bd = maxDist * maxDist;
    for (size_t i = 0; i < boids.size(); i++)
    {
        if (eaten[i]) continue;
        glm::vec3 d = boids[i].pos - from;
        float q = glm::dot(d, d);
        if (q < bd) { bd = q; best = (int)i; outPos = boids[i].pos; }
    }
    return best;
}
