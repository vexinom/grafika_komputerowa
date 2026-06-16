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

static void BuildPath(AxolotlInstance& inst, glm::vec3 center, float radius, float yWobble, float scale, float startDist)
{
    inst.scale = scale;
    inst.dist = startDist;

    const int controlCount = 8;
    std::vector<glm::vec3> control;
    for (int i = 0; i < controlCount; i++)
    {
        float a = 6.28318530718f * (float)i / (float)controlCount;
        control.push_back(center + glm::vec3(cos(a) * radius, sin(a * 2.0f) * yWobble, sin(a) * radius));
    }

    const int perSegment = 40;
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
    inst.cumLen.resize(m);

    for (int i = 0; i < m; i++)
    {
        inst.pathTan[i] = glm::normalize(inst.pathPos[(i + 1) % m] - inst.pathPos[(i - 1 + m) % m]);
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
    opacityTex = LoadTexture((dir + "DefaultMaterial_Opacity.png").c_str());

    animTime = 0.0f;
    instances.resize(3);
    BuildPath(instances[0], glm::vec3(300.0f, 62.0f, 560.0f), 150.0f, 12.0f, 1.6f, 0.0f);
    BuildPath(instances[1], glm::vec3(560.0f, 58.0f, 320.0f), 140.0f, 10.0f, 1.0f, 130.0f);
    BuildPath(instances[2], glm::vec3(360.0f, 55.0f, 760.0f), 120.0f, 9.0f, 0.7f, 260.0f);
}

void Axolotl::Update(float dt)
{
    if (dt > 0.05f) dt = 0.05f;
    animTime = fmod(animTime + dt, 1000.0f);

    float speed = 22.0f;
    for (size_t k = 0; k < instances.size(); k++)
    {
        AxolotlInstance& inst = instances[k];
        int m = (int)inst.pathPos.size();
        inst.dist = fmod(inst.dist + speed * dt, inst.totalLen);

        int idx = 0;
        while (idx < m - 1 && inst.cumLen[idx + 1] < inst.dist) idx++;
        int j = (idx + 1) % m;
        float segLen = (j == 0) ? inst.totalLen - inst.cumLen[idx] : inst.cumLen[j] - inst.cumLen[idx];
        float f = segLen > 1e-5f ? (inst.dist - inst.cumLen[idx]) / segLen : 0.0f;

        glm::vec3 pos = glm::mix(inst.pathPos[idx], inst.pathPos[j], f);
        glm::vec3 T = glm::normalize(glm::mix(inst.pathTan[idx], inst.pathTan[j], f));
        glm::vec3 S = glm::normalize(glm::cross(T, glm::vec3(0.0f, 1.0f, 0.0f)));
        glm::vec3 U = glm::normalize(glm::cross(S, T));

        glm::vec3 Tahead = inst.pathTan[(idx + 4) % m];
        float turn = glm::dot(glm::cross(T, Tahead), U);
        float bank = glm::clamp(turn * 9.0f, -0.6f, 0.6f);
        S = RotateAroundAxis(S, T, bank);
        U = RotateAroundAxis(U, T, bank);

        glm::mat4 basis(1.0f);
        basis[0] = glm::vec4(-S, 0.0f);
        basis[1] = glm::vec4(-T, 0.0f);
        basis[2] = glm::vec4(U, 0.0f);

        float sc = baseScale * inst.scale;
        inst.model = glm::translate(glm::mat4(1.0f), pos) * basis * glm::scale(glm::mat4(1.0f), glm::vec3(sc)) * glm::translate(glm::mat4(1.0f), -bboxCenter);
        inst.currentPos = pos;
        inst.forward = T;
    }
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
