#include "otter.h"
#include "fish.h"
#include "objloader.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <algorithm>

static const float WATER_LEVEL = 400.0f;
static const float TARGET_LENGTH = 65.0f;

static const float HUNGER_MAX  = 100.0f;
static const float HUNGER_RATE = 16.0f;    // gets hungry in ~6s, so it actively hunts
static const float DETECT_RADIUS = 900.0f; // spots fish schools from far away
static const float ATTACK_RADIUS = 70.0f;  // MUST stay larger than HUNT_PRED_RADIUS below

static const float CRUISE_SPEED = 80.0f;
static const float HUNT_SPEED   = 165.0f;  // well above fish top speed (~50) so it can close in

// Scare radius the otter projects onto the fish. While hunting it must be SMALLER
// than ATTACK_RADIUS, otherwise the fish flee-clamp pushes prey out of reach and the
// otter can never actually catch anything.
static const float WANDER_PRED_RADIUS = 140.0f;
static const float HUNT_PRED_RADIUS   = 28.0f;

static const float WORLD_MINX = 650.0f,  WORLD_MAXX = 3550.0f;
static const float WORLD_MINZ = 450.0f,  WORLD_MAXZ = 2950.0f;
static const float WORLD_MINY = 80.0f,   WORLD_MAXY = WATER_LEVEL - 40.0f;
static const glm::vec3 HOME = glm::vec3(1300.0f, 170.0f, 1200.0f);

float Otter::frand(float a, float b) { return a + (b - a) * ((float)rand() / (float)RAND_MAX); }

int Otter::FindClip(const std::string& key) const
{
    for (size_t i = 0; i < clips.size(); i++)
        if (clips[i].name.find(key) != std::string::npos) return (int)i;
    return 0;
}

void Otter::Init()
{
    FILE* f = fopen("assets/models/otter/otter.skel", "rb");
    if (!f) { fprintf(stderr, "Otter: nie udalo sie otworzyc otter.skel\n"); return; }

    char magic[4]; fread(magic, 1, 4, f);
    int version, nverts, nindices, nclips;
    fread(&version, 4, 1, f);
    fread(&nverts, 4, 1, f);
    fread(&nindices, 4, 1, f);
    fread(&jointCount, 4, 1, f);
    fread(&nclips, 4, 1, f);
    float c[3], ext; fread(c, 4, 3, f); fread(&ext, 4, 1, f);
    modelCenter = glm::vec3(c[0], c[1], c[2]);
    extent = ext;

    std::vector<float> interleaved(nverts * 16);
    fread(interleaved.data(), sizeof(float), interleaved.size(), f);
    std::vector<unsigned int> indices(nindices);
    fread(indices.data(), sizeof(unsigned int), indices.size(), f);
    indexCount = nindices;

    clips.resize(nclips);
    for (int i = 0; i < nclips; i++)
    {
        int nameLen; fread(&nameLen, 4, 1, f);
        std::string name(nameLen, '\0'); fread(&name[0], 1, nameLen, f);
        float dur; int nframes;
        fread(&dur, 4, 1, f);
        fread(&nframes, 4, 1, f);
        clips[i].name = name;
        clips[i].duration = dur;
        clips[i].frameCount = nframes;
        clips[i].frames.resize((size_t)nframes * jointCount * 16);
        fread(clips[i].frames.data(), sizeof(float), clips[i].frames.size(), f);
    }
    fclose(f);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, interleaved.size() * sizeof(float), interleaved.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    const int stride = 16 * sizeof(float);
    glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(1); glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2); glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(3); glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, stride, (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(4); glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, stride, (void*)(12 * sizeof(float)));
    glBindVertexArray(0);

    // This otter is a glTF asset: glTF UVs use a top-left origin, so the texture
    // must be loaded WITHOUT the vertical flip that LoadTexture applies by default
    // (the OBJ/legacy convention). With the flip on, the V axis was inverted and
    // the pinkish belly-skin region of the atlas got mapped onto the body, which
    // looked like exposed "organs/skin". flipV=false fixes the mapping.
    albedoTex = LoadTexture("assets/models/otter/Texture/otter.png", false);

    wanderClip = FindClip("swim_fwd_underwater_01");
    fastClip   = FindClip("swim_fwd_surface_fast");
    attackClip = FindClip("swim_surface_dive_fwd");
    idleClip   = FindClip("swim_surface_idle");

    jointUpload.assign(jointCount * 16, 0.0f);
    bufCur.assign(jointCount * 16, 0.0f);
    bufPrev.assign(jointCount * 16, 0.0f);

    srand(7);
    position = HOME;
    hunger = HUNGER_MAX;
    cur.clip = wanderClip; cur.time = 0.0f;
    prev = cur;
    state = WANDER;
    PickWanderTarget(false);
}

void Otter::PickWanderTarget(bool wide)
{
    if (wide)
    {
        wanderTarget = glm::vec3(frand(WORLD_MINX, WORLD_MAXX),
                                 frand(WORLD_MINY, WORLD_MAXY),
                                 frand(WORLD_MINZ, WORLD_MAXZ));
    }
    else
    {
        float a = frand(0.0f, 6.2831853f);
        float r = frand(120.0f, 480.0f);
        wanderTarget = HOME + glm::vec3(cosf(a) * r, frand(-70.0f, 70.0f), sinf(a) * r);
    }
    wanderTarget.x = glm::clamp(wanderTarget.x, WORLD_MINX, WORLD_MAXX);
    wanderTarget.y = glm::clamp(wanderTarget.y, WORLD_MINY, WORLD_MAXY);
    wanderTarget.z = glm::clamp(wanderTarget.z, WORLD_MINZ, WORLD_MAXZ);
}

void Otter::SampleClip(const Player& p, std::vector<float>& out) const
{
    const Clip& cl = clips[p.clip];
    int fc = cl.frameCount;
    int stridef = jointCount * 16;
    if (fc <= 1) { std::copy(cl.frames.begin(), cl.frames.begin() + stridef, out.begin()); return; }

    float dur = (cl.duration > 1e-4f) ? cl.duration : 1.0f;
    float t = fmodf(p.time, dur);
    float fpos = t / dur * (fc - 1);
    int i0 = (int)fpos;
    int i1 = i0 + 1; if (i1 >= fc) i1 = 0;
    float a = fpos - (float)i0;

    const float* A = &cl.frames[(size_t)i0 * stridef];
    const float* B = &cl.frames[(size_t)i1 * stridef];
    for (int k = 0; k < stridef; k++) out[k] = A[k] * (1.0f - a) + B[k] * a;
}

void Otter::SetClip(int clip)
{
    if (clip == cur.clip && manualClip < 0) return;
    prev = cur;
    cur.clip = clip;
    cur.time = 0.0f;
    blendRemaining = blendDuration;
}

void Otter::EnterState(State s)
{
    if (state == s) return;
    state = s;
    stateTimer = 0.0f;
    if (s == WANDER)      SetClip(wanderClip);
    else if (s == ATTACK) { SetClip(attackClip); attackTimer = 0.7f; }
    else                  SetClip(fastClip);
}

void Otter::CycleClip()
{
    if (clips.empty()) return;
    if (manualClip < 0) manualClip = 0;
    else manualClip = (manualClip + 1) % (int)clips.size();
    prev = cur; cur.clip = manualClip; cur.time = 0.0f; blendRemaining = blendDuration;
}

void Otter::StopCycle()
{
    manualClip = -1;
    state = WANDER; stateTimer = 0.0f;
    SetClip(wanderClip);
}

void Otter::Update(float dt, const glm::vec3& cameraPos, Fish& fish)
{
    if (clips.empty()) return;
    if (dt > 0.05f) dt = 0.05f;
    swimPhase += dt * 7.0f;

    cur.time += dt;
    prev.time += dt;
    if (blendRemaining > 0.0f) blendRemaining -= dt;
    stateTimer += dt;

    if (manualClip < 0)
    {
        hunger = glm::max(0.0f, hunger - HUNGER_RATE * dt);

        glm::vec3 desiredVel(0.0f);
        float speed = CRUISE_SPEED;

        if (state == WANDER)
        {
            speed = CRUISE_SPEED;
            glm::vec3 to = wanderTarget - position;
            if (glm::length(to) < 45.0f) PickWanderTarget(false);
            else desiredVel = glm::normalize(to) * speed;
            if (hunger <= 0.0f) { EnterState(HUNGRY); PickWanderTarget(true); }
        }
        else if (state == HUNGRY)
        {
            speed = HUNT_SPEED;
            glm::vec3 found;
            int idx = fish.FindNearestFish(position, DETECT_RADIUS, found);
            if (idx >= 0) { preyIndex = idx; preyPos = found; EnterState(CHASE); }
            else
            {
                glm::vec3 to = wanderTarget - position;
                if (glm::length(to) < 70.0f) PickWanderTarget(true);
                else desiredVel = glm::normalize(to) * speed;
            }
        }
        else if (state == CHASE)
        {
            speed = HUNT_SPEED;
            if (!fish.FishAlive(preyIndex)) { EnterState(HUNGRY); PickWanderTarget(true); }
            else
            {
                preyPos = fish.FishPos(preyIndex);
                glm::vec3 to = preyPos - position;
                float d = glm::length(to);
                if (d < ATTACK_RADIUS) EnterState(ATTACK);
                else if (d > 1e-3f) desiredVel = (to / d) * speed;
            }
        }
        else // ATTACK
        {
            speed = HUNT_SPEED * 0.5f;
            if (fish.FishAlive(preyIndex))
            {
                preyPos = fish.FishPos(preyIndex);
                glm::vec3 to = preyPos - position;
                if (glm::length(to) > 1e-3f) desiredVel = glm::normalize(to) * speed;
            }
            attackTimer -= dt;
            if (attackTimer <= 0.0f)
            {
                if (fish.FishAlive(preyIndex)) fish.EatFish(preyIndex);
                preyIndex = -1;
                hunger = HUNGER_MAX;
                EnterState(WANDER);
                PickWanderTarget(false);
            }
        }

        if (glm::length(desiredVel) > 1.0f)
            desiredVel.y += sinf(swimPhase) * glm::length(desiredVel) * 0.05f;   // subtle dorsoventral glide

        velocity = glm::mix(velocity, desiredVel, glm::clamp(dt * 4.0f, 0.0f, 1.0f));
        position += velocity * dt;

        position.x = glm::clamp(position.x, WORLD_MINX, WORLD_MAXX);
        position.y = glm::clamp(position.y, WORLD_MINY, WORLD_MAXY);
        position.z = glm::clamp(position.z, WORLD_MINZ, WORLD_MAXZ);

        float vlen = glm::length(velocity);
        if (vlen > 3.0f)
            facing = glm::normalize(glm::mix(facing, velocity / vlen, glm::clamp(dt * 5.0f, 0.0f, 1.0f)));

        // Push the fish away hard only while cruising; once chasing, drop the scare
        // radius below ATTACK_RADIUS so the prey stays catchable instead of being
        // shoved out of reach.
        float predR = (state == CHASE || state == ATTACK) ? HUNT_PRED_RADIUS : WANDER_PRED_RADIUS;
        fish.SetPredator(position, predR);
    }

    // orient the whole body along the swim direction (yaw + pitch), otter-style.
    // The model's head is at local +Z, so local +Z must map to the travel
    // direction. The basis MUST be a proper rotation (determinant = +1).
    // The previous version built {right, up, -f}, whose determinant is -1 (a
    // reflection): that mirrored the mesh inside-out -- front faces became back
    // faces, so the otter rendered see-through ("organs/skin/bones") and its
    // apparent swim direction was reversed.
    glm::vec3 fwd = glm::normalize(facing);
    glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    // Near-vertical swimming makes cross(worldUp, fwd) collapse, which snap-rolls the
    // body. Swap to a different reference axis at the poles to keep the basis stable.
    glm::vec3 refUp = (fabsf(fwd.y) > 0.94f) ? glm::vec3(0.0f, 0.0f, 1.0f) : worldUp;
    glm::vec3 right = glm::cross(refUp, fwd);
    float rl = glm::length(right);
    right = (rl > 1e-4f) ? right / rl : glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 up = glm::cross(fwd, right);   // right x up == fwd  =>  det(basis) = +1

    glm::mat4 basis(1.0f);
    basis[0] = glm::vec4(right, 0.0f);
    basis[1] = glm::vec4(up,    0.0f);
    basis[2] = glm::vec4(fwd,   0.0f);   // local +Z (head) -> travel direction

    float scaleFactor = TARGET_LENGTH / extent;
    model = glm::translate(glm::mat4(1.0f), position) * basis;
    model = glm::scale(model, glm::vec3(scaleFactor));
    model = glm::translate(model, -modelCenter);

    SampleClip(cur, bufCur);
    if (blendRemaining > 0.0f)
    {
        SampleClip(prev, bufPrev);
        float a = 1.0f - glm::clamp(blendRemaining / blendDuration, 0.0f, 1.0f);
        for (size_t k = 0; k < jointUpload.size(); k++)
            jointUpload[k] = bufPrev[k] * (1.0f - a) + bufCur[k] * a;
    }
    else
    {
        jointUpload = bufCur;
    }
}

void Otter::Draw(Shader& shader, const glm::mat4& view, const glm::mat4& projection,
                 const glm::vec3& sunDirection, const glm::vec3& cameraPos)
{
    if (indexCount == 0 || clips.empty()) return;

    shader.Use();
    shader.SetMat4("model", model);
    shader.SetMat4("view", view);
    shader.SetMat4("projection", projection);
    shader.SetVec3("sunDirection", sunDirection);
    shader.SetVec3("tint", glm::vec3(1.0f));

    int loc = shader.GetUniformLocation("jointMatrices");
    glUniformMatrix4fv(loc, jointCount, GL_FALSE, jointUpload.data());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, albedoTex);
    shader.SetInt("albedo", 0);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
