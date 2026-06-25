#include "axolotl.h"
#include <cstdio>
#include <string>
#include <map>
#include <tuple>
#include <cmath>
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include "stb_image.h"

static void LoadObj(const std::string& path, std::vector<glm::vec3>& pos, std::vector<glm::vec2>& uvs, std::vector<glm::vec3>& nrm, std::vector<glm::ivec3>& faces)
{
    FILE* file = fopen(path.c_str(), "r");
    if (!file)
    {
        fprintf(stderr, "Failed to open %s\n", path.c_str());
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file))
    {
        if (line[0] == 'v' && line[1] == ' ')
        {
            glm::vec3 v;
            sscanf(line + 2, "%f %f %f", &v.x, &v.y, &v.z);
            pos.push_back(v);
        }
        else if (line[0] == 'v' && line[1] == 't')
        {
            glm::vec2 t;
            sscanf(line + 3, "%f %f", &t.x, &t.y);
            uvs.push_back(t);
        }
        else if (line[0] == 'v' && line[1] == 'n')
        {
            glm::vec3 n;
            sscanf(line + 3, "%f %f %f", &n.x, &n.y, &n.z);
            nrm.push_back(n);
        }
        else if (line[0] == 'f' && line[1] == ' ')
        {
            glm::ivec3 a, b, c;
            sscanf(line + 2, "%d/%d/%d %d/%d/%d %d/%d/%d", &a.x, &a.y, &a.z, &b.x, &b.y, &b.z, &c.x, &c.y, &c.z);
            faces.push_back(a);
            faces.push_back(b);
            faces.push_back(c);
        }
    }
    fclose(file);
}

static unsigned int LoadTexture(const char* path)
{
    stbi_set_flip_vertically_on_load(true);
    int w, h, channels;
    unsigned char* data = stbi_load(path, &w, &h, &channels, 0);
    if (!data)
    {
        fprintf(stderr, "Failed to load %s\n", path);
        return 0;
    }

    GLenum format = (channels == 4) ? GL_RGBA : (channels == 1 ? GL_RED : GL_RGB);
    unsigned int tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(data);
    return tex;
}

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

// ---------------------------------------------------------------------------
// Parallel Transport Frames (rotation-minimizing frames) along a closed spline.
// For B04 the orientation of the creature is taken directly from the PTF normal,
// instead of an ad-hoc cross-product (Frenet-like) frame. The frame is propagated
// by rotating the previous normal by the minimal rotation that maps T[i-1] -> T[i],
// then the closed-loop holonomy (twist mismatch at the seam) is spread evenly over
// the whole loop so the frame is continuous all the way around.
// ---------------------------------------------------------------------------
static void BuildPath(AxolotlInstance& inst, glm::vec3 center, float radius, float yWobble, float scale, float startDist)
{
    inst.scale = scale;
    inst.dist = startDist;
    inst.boost = 0.0f;

    const int controlCount = 8;
    std::vector<glm::vec3> control;
    for (int i = 0; i < controlCount; i++)
    {
        float a = 6.28318530718f * (float)i / (float)controlCount;
        control.push_back(center + glm::vec3(cos(a) * radius, sin(a * 2.0f) * yWobble, sin(a) * radius));
    }

    const int perSegment = 40;
    inst.pathPos.clear();
    for (int i = 0; i < controlCount; i++)
    {
        glm::vec3 p0 = control[(i - 1 + controlCount) % controlCount];
        glm::vec3 p1 = control[i];
        glm::vec3 p2 = control[(i + 1) % controlCount];
        glm::vec3 p3 = control[(i + 2) % controlCount];
        for (int s = 0; s < perSegment; s++)
        {
            inst.pathPos.push_back(CatmullRom(p0, p1, p2, p3, (float)s / (float)perSegment));
        }
    }

    int m = (int)inst.pathPos.size();
    inst.pathTan.resize(m);
    inst.pathNrm.resize(m);
    inst.cumLen.resize(m);

    // central-difference unit tangents (closed loop)
    for (int i = 0; i < m; i++)
    {
        inst.pathTan[i] = glm::normalize(inst.pathPos[(i + 1) % m] - inst.pathPos[(i - 1 + m) % m]);
    }

    // seed normal perpendicular to the first tangent
    glm::vec3 T0 = inst.pathTan[0];
    glm::vec3 up = glm::abs(T0.y) > 0.99f ? glm::vec3(1.0f, 0.0f, 0.0f) : glm::vec3(0.0f, 1.0f, 0.0f);
    inst.pathNrm[0] = glm::normalize(up - T0 * glm::dot(up, T0));

    // propagate the frame with the minimal rotation between successive tangents
    for (int i = 1; i < m; i++)
    {
        glm::vec3 prevT = inst.pathTan[i - 1];
        glm::vec3 curT  = inst.pathTan[i];
        glm::vec3 axis  = glm::cross(prevT, curT);
        float len = glm::length(axis);
        glm::vec3 n = inst.pathNrm[i - 1];
        if (len > 1e-6f)
        {
            axis /= len;
            float angle = atan2(len, glm::dot(prevT, curT));
            n = RotateAroundAxis(n, axis, angle);
        }
        // re-orthonormalise against the current tangent
        inst.pathNrm[i] = glm::normalize(n - curT * glm::dot(n, curT));
    }

    // close the loop: transport once more onto T0 and measure the residual twist
    {
        glm::vec3 prevT = inst.pathTan[m - 1];
        glm::vec3 curT  = inst.pathTan[0];
        glm::vec3 axis  = glm::cross(prevT, curT);
        float len = glm::length(axis);
        glm::vec3 n = inst.pathNrm[m - 1];
        if (len > 1e-6f)
        {
            axis /= len;
            float angle = atan2(len, glm::dot(prevT, curT));
            n = RotateAroundAxis(n, axis, angle);
        }
        n = glm::normalize(n - curT * glm::dot(n, curT));

        // signed angle between the transported normal and the seed normal around T0
        glm::vec3 N0 = inst.pathNrm[0];
        glm::vec3 B0 = glm::cross(curT, N0);
        float defect = atan2(glm::dot(n, B0), glm::dot(n, N0));

        // distribute the correction proportionally to arc-length so the frame closes
        for (int i = 0; i < m; i++)
        {
            float frac = (float)i / (float)m;
            inst.pathNrm[i] = glm::normalize(RotateAroundAxis(inst.pathNrm[i], inst.pathTan[i], -defect * frac));
        }
    }

    inst.cumLen[0] = 0.0f;
    for (int i = 1; i < m; i++)
    {
        inst.cumLen[i] = inst.cumLen[i - 1] + glm::length(inst.pathPos[i] - inst.pathPos[i - 1]);
    }
    inst.totalLen = inst.cumLen[m - 1] + glm::length(inst.pathPos[0] - inst.pathPos[m - 1]);
}

void Axolotl::Init()
{
    std::string dir = "assets/models/Axolotl/";
    std::vector<glm::vec3> pos, nrm;
    std::vector<glm::vec2> uvs;
    std::vector<glm::ivec3> faces;
    LoadObj(dir + "model_0.obj", pos, uvs, nrm, faces);

    std::vector<float> interleaved;
    std::vector<unsigned int> indices;
    int vertexCount = 0;
    glm::vec3 mn(1e9f), mx(-1e9f);

    std::map<std::tuple<int, int, int>, unsigned int> unique;
    for (size_t i = 0; i < faces.size(); i++)
    {
        glm::ivec3 corner = faces[i];
        auto key = std::make_tuple(corner.x, corner.y, corner.z);
        auto found = unique.find(key);
        if (found == unique.end())
        {
            unsigned int id = (unsigned int)vertexCount++;
            unique[key] = id;
            glm::vec3 p = pos[corner.x - 1];
            glm::vec3 n = nrm[corner.z - 1];
            glm::vec2 t = uvs[corner.y - 1];
            interleaved.push_back(p.x); interleaved.push_back(p.y); interleaved.push_back(p.z);
            interleaved.push_back(n.x); interleaved.push_back(n.y); interleaved.push_back(n.z);
            interleaved.push_back(t.x); interleaved.push_back(t.y);
            mn = glm::min(mn, p);
            mx = glm::max(mx, p);
            indices.push_back(id);
        }
        else
        {
            indices.push_back(found->second);
        }
    }

    indexCount = (int)indices.size();
    bboxCenter = (mn + mx) * 0.5f;
    glm::vec3 size = mx - mn;
    float maxExtent = glm::max(size.x, glm::max(size.y, size.z));
    baseScale = 35.0f / maxExtent;
    bodyMinY = mn.y;
    bodyLenY = size.y;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, interleaved.size() * sizeof(float), interleaved.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);

    baseColorTex = LoadTexture((dir + "DefaultMaterial_Base_Color.png").c_str());
    opacityTex   = LoadTexture((dir + "DefaultMaterial_Opacity.png").c_str());
    metallicTex  = LoadTexture((dir + "DefaultMaterial_Metallic.png").c_str());
    roughnessTex = LoadTexture((dir + "DefaultMaterial_Roughness.png").c_str());

    animTime = 0.0f;
    instances.resize(3);
    BuildPath(instances[0], glm::vec3(1024.0f, 55.0f, 1500.0f), 300.0f, 16.0f, 1.6f, 0.0f);
    BuildPath(instances[1], glm::vec3(1300.0f, 30.0f, 1300.0f), 280.0f, 14.0f, 1.0f, 130.0f);
    BuildPath(instances[2], glm::vec3(760.0f, 45.0f, 1480.0f), 250.0f, 12.0f, 0.7f, 260.0f);
}

void Axolotl::Update(float dt)
{
    if (dt > 0.05f) dt = 0.05f;
    animTime = fmod(animTime + dt, 1000.0f);

    float baseSpeed = 22.0f;
    for (size_t k = 0; k < instances.size(); k++)
    {
        AxolotlInstance& inst = instances[k];
        int m = (int)inst.pathPos.size();

        if (inst.boost > 0.0f) inst.boost = glm::max(0.0f, inst.boost - dt);
        float speed = baseSpeed * speedScale + inst.boost * 60.0f;
        if (!paused)
            inst.dist = fmod(inst.dist + speed * dt, inst.totalLen);

        int idx = 0;
        while (idx < m - 1 && inst.cumLen[idx + 1] < inst.dist) idx++;
        int j = (idx + 1) % m;
        float segLen = (j == 0) ? inst.totalLen - inst.cumLen[idx] : inst.cumLen[j] - inst.cumLen[idx];
        float f = segLen > 1e-5f ? (inst.dist - inst.cumLen[idx]) / segLen : 0.0f;

        glm::vec3 pos = glm::mix(inst.pathPos[idx], inst.pathPos[j], f);
        glm::vec3 T = glm::normalize(glm::mix(inst.pathTan[idx], inst.pathTan[j], f));

        // interpolate the parallel-transported normal and re-orthonormalise
        glm::vec3 N = glm::mix(inst.pathNrm[idx], inst.pathNrm[j], f);
        N = N - T * glm::dot(N, T);
        if (glm::length(N) < 1e-5f) N = inst.pathNrm[idx];
        N = glm::normalize(N);
        glm::vec3 B = glm::normalize(glm::cross(T, N));

        // bank into turns for a livelier look (rotates the PTF frame around T)
        glm::vec3 Tahead = inst.pathTan[(idx + 4) % m];
        float turn = glm::dot(glm::cross(T, Tahead), N);
        float bank = glm::clamp(turn * 9.0f, -0.6f, 0.6f);
        N = RotateAroundAxis(N, T, bank);
        B = RotateAroundAxis(B, T, bank);

        // model axes: local +X -> -B (side), local +Y -> -T (body/forward), local +Z -> N (up)
        glm::mat4 basis(1.0f);
        basis[0] = glm::vec4(-B, 0.0f);
        basis[1] = glm::vec4(-T, 0.0f);
        basis[2] = glm::vec4(N, 0.0f);

        float sc = baseScale * inst.scale;
        inst.model = glm::translate(glm::mat4(1.0f), pos) * basis * glm::scale(glm::mat4(1.0f), glm::vec3(sc)) * glm::translate(glm::mat4(1.0f), -bboxCenter);
        inst.currentPos = pos;
        inst.forward = T;
    }
}

glm::mat4 Axolotl::ModelMatrix(const glm::vec3& position, const glm::vec3& forward, float scale) const
{
    glm::vec3 T = forward;
    T.y = 0.0f;

    if (glm::length(T) < 1e-5f)
    {
        T = glm::vec3(0.0f, 0.0f, -1.0f);
    }

    T = glm::normalize(T);

    glm::vec3 N = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 B = glm::normalize(glm::cross(T, N));

    glm::mat4 basis(1.0f);
    basis[0] = glm::vec4(-B, 0.0f);
    basis[1] = glm::vec4(-T, 0.0f);
    basis[2] = glm::vec4(N, 0.0f);

    float sc = baseScale * scale;
    return glm::translate(glm::mat4(1.0f), position)
         * basis
         * glm::scale(glm::mat4(1.0f), glm::vec3(sc))
         * glm::translate(glm::mat4(1.0f), -bboxCenter);
}

void Axolotl::DrawSingle(Shader& shader, const glm::mat4& view, const glm::mat4& projection, const glm::vec3& sunDirection, const glm::vec3& cameraPos, const glm::mat4& lightSpaceMatrix, unsigned int shadowMap, const glm::mat4& model)
{
    shader.Use();
    shader.SetMat4("view", view);
    shader.SetMat4("projection", projection);
    shader.SetMat4("lightSpaceMatrix", lightSpaceMatrix);
    shader.SetVec3("sunDirection", sunDirection);
    shader.SetVec3("cameraPos", cameraPos);
    shader.SetVec3("headlightPos", HeadlightPosition());
    shader.SetVec3("headlightColor", glm::vec3(1.6f, 1.5f, 1.2f));
    shader.SetFloat("time", animTime);
    shader.SetFloat("bodyMinY", bodyMinY);
    shader.SetFloat("bodyLenY", bodyLenY);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, baseColorTex);
    shader.SetInt("baseColor", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, opacityTex);
    shader.SetInt("opacity", 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, metallicTex);
    shader.SetInt("metallicMap", 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, roughnessTex);
    shader.SetInt("roughnessMap", 3);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, shadowMap);
    shader.SetInt("shadowMap", 5);

    glBindVertexArray(VAO);
    shader.SetMat4("model", model);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Axolotl::DrawSingleDepth(Shader& shader, const glm::mat4& lightSpaceMatrix, const glm::mat4& model)
{
    shader.SetMat4("lightSpaceMatrix", lightSpaceMatrix);
    shader.SetMat4("model", model);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Axolotl::Poke(const glm::vec3& from)
{
    float best = 1e18f;
    int bestK = -1;
    for (size_t k = 0; k < instances.size(); k++)
    {
        float d = glm::distance(from, instances[k].currentPos);
        if (d < best) { best = d; bestK = (int)k; }
    }
    if (bestK >= 0) instances[bestK].boost = 1.5f;
}

void Axolotl::Draw(Shader& shader, const glm::mat4& view, const glm::mat4& projection, const glm::vec3& sunDirection, const glm::vec3& cameraPos, const glm::mat4& lightSpaceMatrix, unsigned int shadowMap)
{
    shader.Use();
    shader.SetMat4("view", view);
    shader.SetMat4("projection", projection);
    shader.SetMat4("lightSpaceMatrix", lightSpaceMatrix);
    shader.SetVec3("sunDirection", sunDirection);
    shader.SetVec3("cameraPos", cameraPos);
    shader.SetVec3("headlightPos", HeadlightPosition());
    shader.SetVec3("headlightColor", glm::vec3(1.6f, 1.5f, 1.2f));
    shader.SetFloat("time", animTime);
    shader.SetFloat("bodyMinY", bodyMinY);
    shader.SetFloat("bodyLenY", bodyLenY);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, baseColorTex);
    shader.SetInt("baseColor", 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, opacityTex);
    shader.SetInt("opacity", 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, metallicTex);
    shader.SetInt("metallicMap", 2);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, roughnessTex);
    shader.SetInt("roughnessMap", 3);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, shadowMap);
    shader.SetInt("shadowMap", 5);

    glBindVertexArray(VAO);
    for (size_t k = 0; k < instances.size(); k++)
    {
        shader.SetMat4("model", instances[k].model);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    }
    glBindVertexArray(0);
}

void Axolotl::DrawDepth(Shader& shader, const glm::mat4& lightSpaceMatrix)
{
    shader.SetMat4("lightSpaceMatrix", lightSpaceMatrix);
    glBindVertexArray(VAO);
    for (size_t k = 0; k < instances.size(); k++)
    {
        shader.SetMat4("model", instances[k].model);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    }
    glBindVertexArray(0);
}

glm::vec3 Axolotl::NearestTo(const glm::vec3& point) const
{
    glm::vec3 best = instances[0].currentPos;
    float bestDist = glm::distance(point, best);
    for (size_t k = 1; k < instances.size(); k++)
    {
        float d = glm::distance(point, instances[k].currentPos);
        if (d < bestDist)
        {
            bestDist = d;
            best = instances[k].currentPos;
        }
    }
    return best;
}

glm::vec3 Axolotl::HeadlightPosition() const
{
    return instances[0].currentPos + instances[0].forward * 22.0f;
}
