#include "islandPalms.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <tuple>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "objloader.h"
#include "stb_image.h"

static float frand()
{
    return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

static float frand(float a, float b)
{
    return a + (b - a) * frand();
}

static glm::ivec3 ParseCornerPalm(const char* s)
{
    glm::ivec3 c(0, 0, 0);

    if (sscanf(s, "%d/%d/%d", &c.x, &c.y, &c.z) == 3) return c;

    c = glm::ivec3(0);
    if (sscanf(s, "%d//%d", &c.x, &c.z) == 2) return c;

    c = glm::ivec3(0);
    if (sscanf(s, "%d/%d", &c.x, &c.y) == 2) return c;

    c = glm::ivec3(0);
    sscanf(s, "%d", &c.x);
    return c;
}

struct PalmObjPart
{
    std::vector<float> interleaved;
    std::vector<unsigned int> indices;
    glm::vec3 minBounds = glm::vec3(1e9f);
    glm::vec3 maxBounds = glm::vec3(-1e9f);
};

static bool LoadObjMaterialPart(const std::string& path,
                                const std::string& wantedMaterial,
                                PalmObjPart& out)
{
    FILE* file = fopen(path.c_str(), "r");
    if (!file)
    {
        fprintf(stderr, "IslandPalms: nie udalo sie otworzyc %s\n", path.c_str());
        return false;
    }

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;
    std::vector<glm::ivec3> faceCorners;

    std::string currentMaterial;

    char line[2048];

    while (fgets(line, sizeof(line), file))
    {
        if (strncmp(line, "usemtl ", 7) == 0)
        {
            char materialName[256] = { 0 };
            sscanf(line + 7, "%255s", materialName);
            currentMaterial = materialName;
        }
        else if (line[0] == 'v' && line[1] == ' ')
        {
            glm::vec3 v;
            sscanf(line + 2, "%f %f %f", &v.x, &v.y, &v.z);
            positions.push_back(v);
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
            normals.push_back(n);
        }
        else if (line[0] == 'f' && line[1] == ' ')
        {
            if (currentMaterial != wantedMaterial)
                continue;

            std::vector<glm::ivec3> polygonCorners;
            char* p = line + 2;
            char token[256];

            while (*p)
            {
                while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
                    p++;

                if (!*p)
                    break;

                int n = 0;
                while (*p && *p != ' ' && *p != '\t' && *p != '\r' && *p != '\n' && n < 255)
                    token[n++] = *p++;

                token[n] = '\0';
                polygonCorners.push_back(ParseCornerPalm(token));
            }

            for (size_t i = 2; i < polygonCorners.size(); i++)
            {
                faceCorners.push_back(polygonCorners[0]);
                faceCorners.push_back(polygonCorners[i - 1]);
                faceCorners.push_back(polygonCorners[i]);
            }
        }
    }

    fclose(file);

    if (positions.empty() || faceCorners.empty())
    {
        fprintf(stderr, "IslandPalms: material %s nie ma geometrii\n", wantedMaterial.c_str());
        return false;
    }

    bool hasNormals = !normals.empty();
    bool hasUVs = !uvs.empty();

    std::map<std::tuple<int, int, int>, unsigned int> uniqueVertices;

    for (const glm::ivec3& corner : faceCorners)
    {
        auto key = std::make_tuple(corner.x, corner.y, corner.z);
        auto found = uniqueVertices.find(key);

        if (found == uniqueVertices.end())
        {
            unsigned int id = static_cast<unsigned int>(uniqueVertices.size());
            uniqueVertices[key] = id;

            glm::vec3 p = positions[corner.x - 1];
            glm::vec3 n = (hasNormals && corner.z > 0) ? normals[corner.z - 1] : glm::vec3(0.0f, 1.0f, 0.0f);
            glm::vec2 uv = (hasUVs && corner.y > 0) ? uvs[corner.y - 1] : glm::vec2(0.0f);

            out.interleaved.push_back(p.x);
            out.interleaved.push_back(p.y);
            out.interleaved.push_back(p.z);

            out.interleaved.push_back(n.x);
            out.interleaved.push_back(n.y);
            out.interleaved.push_back(n.z);

            out.interleaved.push_back(uv.x);
            out.interleaved.push_back(uv.y);

            out.indices.push_back(id);

            out.minBounds = glm::min(out.minBounds, p);
            out.maxBounds = glm::max(out.maxBounds, p);
        }
        else
        {
            out.indices.push_back(found->second);
        }
    }

    return true;
}

static void CreateMeshFromPart(const PalmObjPart& part,
                               unsigned int& VAO,
                               unsigned int& VBO,
                               unsigned int& EBO)
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER,
                 part.interleaved.size() * sizeof(float),
                 part.interleaved.data(),
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 part.indices.size() * sizeof(unsigned int),
                 part.indices.data(),
                 GL_STATIC_DRAW);

    // position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

    // normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

    // uv
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    glBindVertexArray(0);
}

void IslandPalms::Init()
{
    const std::string objPath = "assets/models/palm/palm.obj";

    struct MaterialSource
    {
        const char* materialName;
        const char* texturePath;
    };

    const MaterialSource sources[] =
    {
        { "Palm_L_Mat",  "assets/models/palm/textures/Palm_L_Mat_baseColor.png"  },
        { "Palm_T_Mat",  "assets/models/palm/textures/Palm_T_Mat_baseColor.png"  },
        { "PalmD_T_Mat", "assets/models/palm/textures/PalmD_T_Mat_baseColor.png" }
    };

    glm::vec3 globalMin(1e9f);
    glm::vec3 globalMax(-1e9f);

    for (const MaterialSource& src : sources)
    {
        PalmObjPart part;

        if (!LoadObjMaterialPart(objPath, src.materialName, part))
            continue;

        Mesh mesh;
        mesh.indexCount = static_cast<int>(part.indices.size());
        mesh.albedoTex = LoadTexture(src.texturePath, true);
        CreateMeshFromPart(part, mesh.VAO, mesh.VBO, mesh.EBO);

        globalMin = glm::min(globalMin, part.minBounds);
        globalMax = glm::max(globalMax, part.maxBounds);

        meshes.push_back(mesh);
    }

    if (meshes.empty())
    {
        fprintf(stderr, "IslandPalms: nie zaladowano zadnej czesci palmy\n");
        return;
    }

    glm::vec3 modelCenter = (globalMin + globalMax) * 0.5f;
    float modelMinY = globalMin.y;
    float modelHeight = glm::max(0.01f, globalMax.y - globalMin.y);

    stbi_set_flip_vertically_on_load(true);

    int w = 0;
    int h = 0;
    int n = 0;

    unsigned char* hm = stbi_load("assets/worldmap.png", &w, &h, &n, 1);

    float mapW = static_cast<float>(w ? w : 2048);
    float mapH = static_cast<float>(h ? h : 2048);

    auto terrainHeight = [&](float x, float z) -> float
    {
        if (!hm)
            return 420.0f;

        int xi = static_cast<int>(glm::clamp(x, 0.0f, mapW - 1.0f));
        int zi = static_cast<int>(glm::clamp(z, 0.0f, mapH - 1.0f));

        float s = hm[zi * w + xi] / 255.0f;

        // To jest wysokosc faktycznego terenu uzywana tez przy reefach.
        return s * 500.0f;
    };

    srand(2606);

    const int targetPalms = 28;
    const float minIslandHeight = 400.0f;

    int attempts = 0;

    while (static_cast<int>(instances.size()) < targetPalms && attempts < 6000)
    {
        attempts++;

        float x = frand(80.0f, mapW - 80.0f);
        float z = frand(80.0f, mapH - 80.0f);
        float y = terrainHeight(x, z);

        // Stawiamy tylko na wyspach / nad woda.
        if (y < minIslandHeight)
            continue;

        float targetHeight = frand(95.0f, 145.0f);
        float scale = targetHeight / modelHeight;
        float rot = frand(0.0f, 6.2831853f);

        Instance inst;
        inst.pos = glm::vec3(x, y + 0.5f, z);

        glm::mat4 model(1.0f);
        model = glm::translate(model, inst.pos);
        model = glm::rotate(model, rot, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(scale));
        model = glm::translate(model, glm::vec3(-modelCenter.x, -modelMinY, -modelCenter.z));

        inst.model = model;
        instances.push_back(inst);
    }

    if (hm)
        stbi_image_free(hm);

    fprintf(stderr, "IslandPalms: loaded %zu mesh parts, placed %zu palms\n", meshes.size(), instances.size());
}

void IslandPalms::Draw(Shader& shader,
                       const glm::mat4& view,
                       const glm::mat4& projection,
                       const glm::vec3& sunDirection,
                       const glm::vec3& cameraPos)
{
    if (meshes.empty() || instances.empty())
        return;

    shader.Use();

    shader.SetMat4("view", view);
    shader.SetMat4("projection", projection);
    shader.SetVec3("sunDirection", sunDirection);
    shader.SetVec3("cameraPos", cameraPos);
    shader.SetInt("albedo", 0);

    glActiveTexture(GL_TEXTURE0);

    for (const Instance& inst : instances)
    {
        shader.SetMat4("model", inst.model);

        for (const Mesh& mesh : meshes)
        {
            glBindTexture(GL_TEXTURE_2D, mesh.albedoTex);
            glBindVertexArray(mesh.VAO);
            glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, 0);
        }
    }

    glBindVertexArray(0);
}

void IslandPalms::DrawDepth(Shader& shader, const glm::mat4& lightSpaceMatrix)
{
    if (meshes.empty() || instances.empty())
        return;

    shader.Use();
    shader.SetMat4("lightSpaceMatrix", lightSpaceMatrix);

    for (const Instance& inst : instances)
    {
        shader.SetMat4("model", inst.model);

        for (const Mesh& mesh : meshes)
        {
            glBindVertexArray(mesh.VAO);
            glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, 0);
        }
    }

    glBindVertexArray(0);
}
