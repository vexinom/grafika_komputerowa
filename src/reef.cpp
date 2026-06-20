#include "reef.h"
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <map>
#include <array>
#include <algorithm>
#include <tuple>
#include <cmath>
#include <cstdlib>
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include "stb_image.h"

static float frand() { return (float)rand() / (float)RAND_MAX; }
static float frand(float a, float b) { return a + (b - a) * frand(); }

enum Cat { ROCK, CORAL_TABLE, CORAL_PLATE, CORAL_WHIP, FEATHER, URCHIN, SHELL, NUM_CAT };

#ifndef GL_TEXTURE_MAX_ANISOTROPY_EXT
#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
#endif
#ifndef GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
#endif

static unsigned int WhiteTexture()
{
    unsigned int tex; glGenTextures(1, &tex); glBindTexture(GL_TEXTURE_2D, tex);
    unsigned char px[4] = { 255, 255, 255, 255 };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, px);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    return tex;
}

static unsigned int LoadTGA(const std::string& path)
{
    stbi_set_flip_vertically_on_load(true);
    int w, h, ch;
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &ch, 0);
    if (!data)
    {
        fprintf(stderr, "Reef: failed to load texture %s  -> using white fallback\n", path.c_str());
        return WhiteTexture();   // keep the prop visible even if the texture is missing
    }
    GLenum format = (ch == 4) ? GL_RGBA : (ch == 1 ? GL_RED : GL_RGB);
    unsigned int tex; glGenTextures(1, &tex); glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    float maxA = 1.0f; glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxA);
    if (maxA > 1.0f) glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxA < 8.0f ? maxA : 8.0f);
    stbi_image_free(data);
    return tex;
}

struct ReefObjData
{
    std::vector<float> interleaved;
    std::vector<unsigned int> indices;
    std::string material;
    glm::vec3 center = glm::vec3(0.0f);
    float minY = 0.0f;
    float invExtent = 1.0f;
};

static bool ParseObjLOD0(const std::string& path, ReefObjData& out)
{
    FILE* file = fopen(path.c_str(), "r");
    if (!file) { fprintf(stderr, "Reef: failed to open %s\n", path.c_str()); return false; }

    std::vector<glm::vec3> pos, nrm; std::vector<glm::vec2> uvs; std::vector<glm::ivec3> faces;
    bool inLOD0 = true; char line[1024];
    while (fgets(line, sizeof(line), file))
    {
        if ((line[0] == 'o' || line[0] == 'g') && line[1] == ' ')
        {
            std::string nm(line + 2);
            bool hasLOD = nm.find("LOD") != std::string::npos;
            bool isLOD0 = nm.find("LOD0") != std::string::npos;
            inLOD0 = (!hasLOD) || isLOD0;
        }
        else if (inLOD0 && strncmp(line, "usemtl ", 7) == 0)
        {
            if (out.material.empty()) { char buf[128] = { 0 }; sscanf(line + 7, "%127s", buf); out.material = buf; }
        }
        else if (line[0] == 'v' && line[1] == ' ') { glm::vec3 v; sscanf(line + 2, "%f %f %f", &v.x, &v.y, &v.z); pos.push_back(v); }
        else if (line[0] == 'v' && line[1] == 't') { glm::vec2 t; sscanf(line + 3, "%f %f", &t.x, &t.y); uvs.push_back(t); }
        else if (line[0] == 'v' && line[1] == 'n') { glm::vec3 n; sscanf(line + 3, "%f %f %f", &n.x, &n.y, &n.z); nrm.push_back(n); }
        else if (line[0] == 'f' && line[1] == ' ' && inLOD0)
        {
            glm::ivec3 a(0), b(0), c(0);
            if (sscanf(line + 2, "%d/%d/%d %d/%d/%d %d/%d/%d", &a.x, &a.y, &a.z, &b.x, &b.y, &b.z, &c.x, &c.y, &c.z) == 9)
            { faces.push_back(a); faces.push_back(b); faces.push_back(c); }
        }
    }
    fclose(file);
    if (pos.empty() || faces.empty()) return false;

    bool hasN = !nrm.empty(), hasUV = !uvs.empty();
    glm::vec3 mn(1e9f), mx(-1e9f);
    std::map<std::tuple<int, int, int>, unsigned int> unique; int vc = 0;
    for (size_t i = 0; i < faces.size(); i++)
    {
        glm::ivec3 corner = faces[i];
        auto key = std::make_tuple(corner.x, corner.y, corner.z);
        auto found = unique.find(key);
        if (found == unique.end())
        {
            unsigned int id = (unsigned int)vc++; unique[key] = id;
            glm::vec3 p = pos[corner.x - 1];
            glm::vec3 n = hasN ? nrm[corner.z - 1] : glm::vec3(0, 1, 0);
            glm::vec2 t = hasUV ? uvs[corner.y - 1] : glm::vec2(0);
            out.interleaved.push_back(p.x); out.interleaved.push_back(p.y); out.interleaved.push_back(p.z);
            out.interleaved.push_back(n.x); out.interleaved.push_back(n.y); out.interleaved.push_back(n.z);
            out.interleaved.push_back(t.x); out.interleaved.push_back(t.y);
            mn = glm::min(mn, p); mx = glm::max(mx, p);
            out.indices.push_back(id);
        }
        else out.indices.push_back(found->second);
    }
    out.center = (mn + mx) * 0.5f; out.minY = mn.y;
    glm::vec3 size = mx - mn; float me = glm::max(size.x, glm::max(size.y, size.z));
    out.invExtent = (me > 1e-4f) ? (1.0f / me) : 1.0f;
    return true;
}

static std::array<unsigned int, 3> TexSetFor(const std::string& mat, std::map<std::string, std::array<unsigned int, 3>>& cache)
{
    auto it = cache.find(mat); if (it != cache.end()) return it->second;
    const std::string dir = "assets/models/fauna/marineBiome_blender/Textures/";
    std::string stem, albedoStem;
    if      (mat == "MAT_Sea_Barnacle") { stem = "Barnacles";    albedoStem = "Barnacles"; }
    else if (mat == "MAT_Coral_Table")  { stem = "Coral_Table";  albedoStem = "Coral_Table"; }
    else if (mat == "MAT_Coral_Plate")  { stem = "Plate_Coral";  albedoStem = "Plate_Coral"; }
    else if (mat == "MAT_Coral_Whip")   { stem = "Coral_Whip";   albedoStem = "Coral_Whip"; }
    else if (mat == "MAT_Feather_Star") { stem = "Feather_Star"; albedoStem = "Feather_Star"; }
    else if (mat == "MAT_Urchin")       { stem = "Urchin";       albedoStem = "Urchin_DefaultColor"; }
    else                                { stem = "seaShells";    albedoStem = "seaShells"; }
    std::array<unsigned int, 3> set;
    set[0] = LoadTGA(dir + "TEX_" + albedoStem + "_BaseColor.tga");
    set[1] = LoadTGA(dir + "TEX_" + stem + "_Normal.tga");
    set[2] = LoadTGA(dir + "TEX_" + stem + "_OcclusionRoughnessMetallic.tga");
    cache[mat] = set;
    return set;
}

static int catFromMaterial(const std::string& mat)
{
    if (mat == "MAT_Sea_Barnacle") return ROCK;
    if (mat == "MAT_Coral_Table")  return CORAL_TABLE;
    if (mat == "MAT_Coral_Plate")  return CORAL_PLATE;
    if (mat == "MAT_Coral_Whip")   return CORAL_WHIP;
    if (mat == "MAT_Feather_Star") return FEATHER;
    if (mat == "MAT_Urchin")       return URCHIN;
    return SHELL;
}

static void sizeFor(int cat, float& size)
{
    switch (cat)
    {
        case ROCK:        size = frand(55.0f, 120.0f); break;
        case CORAL_TABLE: size = frand(45.0f, 95.0f);  break;
        case CORAL_PLATE: size = frand(50.0f, 100.0f); break;
        case CORAL_WHIP:  size = frand(55.0f, 120.0f); break;   // tall upright coral "plants"
        case FEATHER:     size = frand(20.0f, 42.0f);  break;
        case URCHIN:      size = frand(9.0f, 22.0f);   break;
        default:          size = frand(5.0f, 15.0f);   break; // SHELL
    }
}

// preferred seabed-height band per category -> depth zonation
static void bandFor(int cat, float& lo, float& hi)
{
    switch (cat)
    {
        case ROCK:        lo = -680.0f; hi = 40.0f;  break;
        case CORAL_TABLE: lo = -320.0f; hi = 40.0f;  break;
        case CORAL_PLATE: lo = -320.0f; hi = 30.0f;  break;
        case CORAL_WHIP:  lo = -340.0f; hi = 20.0f;  break;
        case FEATHER:     lo = -420.0f; hi = -8.0f;  break;
        case URCHIN:      lo = -500.0f; hi = 35.0f;  break;
        default:          lo = -25.0f;  hi = 72.0f;  break; // SHELL -> sandy shelf/shallows
    }
}

void Reef::Init()
{
    const std::string dir = "assets/models/fauna/obj/";
    const char* files[] = {
        "SM_Barnacle_Rock_01", "SM_Barnacle_Rock_02",
        "SM_Coral_Table_Acropora_01", "SM_Coral_Table_Acropora_02", "SM_Coral_Table_Acropora_03",
        "SM_Coral_Table_Acropora_04", "SM_Coral_Table_Acropora_05", "SM_Coral_Table_Acropora_06",
        "SM_Coral_Table_Acropora_07", "SM_Coral_Table_Acropora_08",
        "SM_Coral_Plate_Montipora_01", "SM_Coral_Plate_Montipora_03",
        "SM_Coral_Whip_01", "SM_Coral_Whip_02",
        "SM_Feather_Star_01", "SM_Feather_Star_02",
        "SM_Urchin_01", "SM_Urchin_02", "SM_Urchin_03", "SM_Urchin_04",
        "SM_Urchin_Group_01", "SM_Urchin_Group_02", "SM_Urchin_Group_03",
        "SM_Starfish_01", "SM_Starfish_02", "SM_Starfish_03",
        "SM_Sand_Dollar_01", "SM_Sand_Dollar_02", "SM_Sand_Dollar_03", "SM_Sand_Dollar_04",
        "SM_Clam", "SM_Mussel_01", "SM_Mussel_02", "SM_Mussel_03", "SM_Mussel_Group_01",
        "SM_Limpet_01", "SM_Limpet_02", "SM_Chiton_01", "SM_Chiton_02",
        "SM_Seaslug_01", "SM_Seaslug_02",
        "SM_Barnacle_01", "SM_Barnacle_02", "SM_Barnacle_03", "SM_Barnacle_Group_01",
    };
    const int fileCount = (int)(sizeof(files) / sizeof(files[0]));

    std::map<std::string, std::array<unsigned int, 3>> texCache;
    std::vector<int> meshCat;
    std::vector<glm::vec3> meshCenter;
    std::vector<float> meshMinY, meshInvExt;
    std::vector<std::vector<int>> catMeshes(NUM_CAT);

    for (int i = 0; i < fileCount; i++)
    {
        ReefObjData data;
        if (!ParseObjLOD0(dir + files[i] + ".obj", data)) continue;

        Mesh m; m.indexCount = (int)data.indices.size();
        glGenVertexArrays(1, &m.VAO); glGenBuffers(1, &m.VBO); glGenBuffers(1, &m.EBO);
        glBindVertexArray(m.VAO);
        glBindBuffer(GL_ARRAY_BUFFER, m.VBO);
        glBufferData(GL_ARRAY_BUFFER, data.interleaved.size() * sizeof(float), data.interleaved.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.indices.size() * sizeof(unsigned int), data.indices.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1); glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2); glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindVertexArray(0);

        std::array<unsigned int, 3> set = TexSetFor(data.material, texCache);
        m.albedoTex = set[0]; m.normalTex = set[1]; m.ormTex = set[2];

        int cat = catFromMaterial(data.material);
        int mi = (int)meshes.size();
        meshes.push_back(m);
        meshCat.push_back(cat); meshCenter.push_back(data.center); meshMinY.push_back(data.minY); meshInvExt.push_back(data.invExtent);
        catMeshes[cat].push_back(mi);
    }
    if (meshes.empty()) return;

    stbi_set_flip_vertically_on_load(true);
    int w = 0, h = 0, n = 0;
    unsigned char* hm = stbi_load("assets/worldmap.png", &w, &h, &n, 1);
    const float waterLevel = 80.0f;
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

    auto realFloor = [&](float x, float z) -> float
    {
        float s = 0.0f;
        if (hm)
        {
            int xi = (int)glm::clamp(x, 0.0f, (float)w - 1.0f);
            int zi = (int)glm::clamp(z, 0.0f, (float)h - 1.0f);
            s = hm[zi * w + xi] / 255.0f;
        }
        return s * 500.0f;
    };

    int weight[NUM_CAT] = { 0 };
    weight[ROCK] = 10; weight[CORAL_TABLE] = 16; weight[CORAL_PLATE] = 8;
    weight[CORAL_WHIP] = 18; weight[FEATHER] = 6; weight[URCHIN] = 14; weight[SHELL] = 34;
    int weightTotal = 0;
    for (int c = 0; c < NUM_CAT; c++) if (!catMeshes[c].empty()) weightTotal += weight[c];
    if (weightTotal == 0) { if (hm) stbi_image_free(hm); return; }

    srand(2024);
    float mapW = (float)(w ? w : 2048), mapH = (float)(h ? h : 2048);

    // reef hotspots, each remembering its seabed height, spanning shelf + slope
    struct Cl { glm::vec2 p; float y; };
    std::vector<Cl> clusters;
    {
        int a = 0;
        while ((int)clusters.size() < 70 && a < 70 * 90)
        {
            a++;
            float cx = frand(120.0f, mapW - 120.0f), cz = frand(120.0f, mapH - 120.0f);
            float fy = seabed(cx, cz);
            if (fy > 72.0f || fy < -560.0f) continue;
            clusters.push_back({ glm::vec2(cx, cz), fy });
        }
    }

    const int target = 620;
    int attempts = 0;
    while ((int)instances.size() < target && attempts < target * 90)
    {
        attempts++;
        int r = rand() % weightTotal;
        int cat = SHELL, acc = 0;
        for (int c = 0; c < NUM_CAT; c++) { if (catMeshes[c].empty()) continue; acc += weight[c]; if (r < acc) { cat = c; break; } }

        float lo, hi; bandFor(cat, lo, hi);

        // find a spot whose seabed height lands in THIS category's depth band
        float x = 0.0f, z = 0.0f, floorY = 0.0f;
        bool ok = false;
        for (int tryi = 0; tryi < 8 && !ok; tryi++)
        {
            if (!clusters.empty() && frand() < 0.85f)
            {
                const Cl* pick = nullptr;
                for (int s = 0; s < 10; s++) { const Cl& cc = clusters[rand() % clusters.size()]; if (cc.y >= lo && cc.y <= hi) { pick = &cc; break; } }
                if (!pick) continue;
                float spread = (cat == SHELL || cat == URCHIN) ? frand(10.0f, 130.0f) : frand(4.0f, 48.0f);
                float ang = frand(0.0f, 6.2831f), rad = frand(0.0f, spread);
                x = pick->p.x + cosf(ang) * rad; z = pick->p.y + sinf(ang) * rad;
            }
            else { x = frand(70.0f, mapW - 70.0f); z = frand(70.0f, mapH - 70.0f); }
            x = glm::clamp(x, 2.0f, mapW - 2.0f); z = glm::clamp(z, 2.0f, mapH - 2.0f);
            floorY = seabed(x, z);
            if (floorY <= waterLevel - 2.0f && floorY >= lo && floorY <= hi) ok = true;
        }
        if (!ok) continue;

        int mi = catMeshes[cat][rand() % catMeshes[cat].size()];
        float size; sizeFor(cat, size);
        float rot = frand(0.0f, 6.2831f);

        float ry = realFloor(x, z);
        if (ry > 395.0f) continue;          // nie stawiaj korali nad powierzchnia wody

        Instance inst;
        inst.mesh = mi;
        inst.pos = glm::vec3(x, ry - 0.5f, z);
        inst.tint = glm::vec3(frand(0.86f, 1.14f));
        inst.rough = 1.0f; inst.metal = 0.0f;

        glm::vec3 c = meshCenter[mi]; float my = meshMinY[mi];
        glm::mat4 model = glm::translate(glm::mat4(1.0f), inst.pos);
        model = glm::rotate(model, rot, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(size * meshInvExt[mi]));
        model = glm::translate(model, glm::vec3(-c.x, -my, -c.z));
        inst.model = model;
        instances.push_back(inst);
    }

    if (hm) stbi_image_free(hm);
    std::sort(instances.begin(), instances.end(), [](const Instance& a, const Instance& b) { return a.mesh < b.mesh; });
}

void Reef::Draw(Shader& shader, const glm::mat4& view, const glm::mat4& projection,
                const glm::vec3& sunDirection, const glm::vec3& cameraPos,
                const glm::mat4& lightSpaceMatrix, unsigned int shadowMap,
                const glm::vec3& headlightPos, const glm::vec3& headlightColor)
{
    if (meshes.empty()) return;
    shader.Use();
    shader.SetMat4("view", view); shader.SetMat4("projection", projection); shader.SetMat4("lightSpaceMatrix", lightSpaceMatrix);
    shader.SetVec3("sunDirection", sunDirection); shader.SetVec3("cameraPos", cameraPos);
    shader.SetVec3("headlightPos", headlightPos); shader.SetVec3("headlightColor", headlightColor);
    shader.SetInt("albedoMap", 0); shader.SetInt("normalMap", 1); shader.SetInt("ormMap", 2); shader.SetInt("shadowMap", 5);
    glActiveTexture(GL_TEXTURE5); glBindTexture(GL_TEXTURE_2D, shadowMap);

    int lastMesh = -1;
    for (size_t i = 0; i < instances.size(); i++)
    {
        const Instance& inst = instances[i];
        if (glm::distance(cameraPos, inst.pos) > 1300.0f) continue;
        if (inst.mesh != lastMesh)
        {
            const Mesh& mref = meshes[inst.mesh];
            glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, mref.albedoTex);
            glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, mref.normalTex);
            glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, mref.ormTex);
            glBindVertexArray(mref.VAO);
            lastMesh = inst.mesh;
        }
        shader.SetMat4("model", inst.model);
        shader.SetVec3("albedoTint", inst.tint);
        glDrawElements(GL_TRIANGLES, meshes[inst.mesh].indexCount, GL_UNSIGNED_INT, 0);
    }
    glBindVertexArray(0);
}

void Reef::DrawDepth(Shader& shader, const glm::mat4& lightSpaceMatrix)
{
    if (meshes.empty()) return;
    shader.SetMat4("lightSpaceMatrix", lightSpaceMatrix);
    int lastMesh = -1;
    for (size_t i = 0; i < instances.size(); i++)
    {
        if (instances[i].mesh != lastMesh) { glBindVertexArray(meshes[instances[i].mesh].VAO); lastMesh = instances[i].mesh; }
        shader.SetMat4("model", instances[i].model);
        glDrawElements(GL_TRIANGLES, meshes[instances[i].mesh].indexCount, GL_UNSIGNED_INT, 0);
    }
    glBindVertexArray(0);
}
