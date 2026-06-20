#include "objloader.h"

#include <cstdio>
#include <cstring>
#include <map>
#include <tuple>
#include <glad/glad.h>
#include "stb_image.h"  


unsigned int WhiteTexture()
{
    unsigned int tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    unsigned char px[4] = { 255, 255, 255, 255 };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, px);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    return tex;
}

unsigned int LoadTexture(const std::string& path, bool flipV)
{
    stbi_set_flip_vertically_on_load(flipV ? 1 : 0);
    int w, h, ch;
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &ch, 0);
    if (!data)
    {
        fprintf(stderr, "LoadTexture: nie udalo sie wczytac %s -> bialy fallback\n", path.c_str());
        return WhiteTexture();
    }
    GLenum format = (ch == 4) ? GL_RGBA : (ch == 1 ? GL_RED : GL_RGB);
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

static glm::ivec3 ParseCorner(const char* s)
{
    glm::ivec3 c(0, 0, 0);
    // probujemy od najpelniejszego formatu do najprostszego
    if (sscanf(s, "%d/%d/%d", &c.x, &c.y, &c.z) == 3) return c;
    c = glm::ivec3(0);
    if (sscanf(s, "%d//%d", &c.x, &c.z) == 2)         return c;
    c = glm::ivec3(0);
    if (sscanf(s, "%d/%d", &c.x, &c.y) == 2)          return c;
    c = glm::ivec3(0);
    sscanf(s, "%d", &c.x);
    return c;
}

bool LoadObj(const std::string& path, ObjMesh& out)
{
    FILE* file = fopen(path.c_str(), "r");
    if (!file)
    {
        fprintf(stderr, "LoadObj: nie udalo sie otworzyc %s\n", path.c_str());
        return false;
    }

    std::vector<glm::vec3> pos, nrm;
    std::vector<glm::vec2> uvs;
    std::vector<glm::ivec3> faceCorners;   // wierzcholki po triangulacji (3 na trojkat)

    char line[1024];
    while (fgets(line, sizeof(line), file))
    {
        if (line[0] == 'v' && line[1] == ' ')
        {
            glm::vec3 v; sscanf(line + 2, "%f %f %f", &v.x, &v.y, &v.z); pos.push_back(v);
        }
        else if (line[0] == 'v' && line[1] == 't')
        {
            glm::vec2 t; sscanf(line + 3, "%f %f", &t.x, &t.y); uvs.push_back(t);
        }
        else if (line[0] == 'v' && line[1] == 'n')
        {
            glm::vec3 n; sscanf(line + 3, "%f %f %f", &n.x, &n.y, &n.z); nrm.push_back(n);
        }
        else if (line[0] == 'f' && line[1] == ' ')
        {
            // rozbij linie na tokeny (rogi wielokata)
            std::vector<glm::ivec3> poly;
            char* p = line + 2;
            char token[128];
            while (*p)
            {
                while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
                if (!*p) break;
                int n = 0;
                while (*p && *p != ' ' && *p != '\t' && *p != '\r' && *p != '\n' && n < 127)
                    token[n++] = *p++;
                token[n] = '\0';
                poly.push_back(ParseCorner(token));
            }
            // triangulacja wachlarzem: (0,1,2), (0,2,3), ...
            for (size_t i = 2; i < poly.size(); i++)
            {
                faceCorners.push_back(poly[0]);
                faceCorners.push_back(poly[i - 1]);
                faceCorners.push_back(poly[i]);
            }
        }
    }
    fclose(file);

    if (pos.empty() || faceCorners.empty()) return false;

    bool hasN = !nrm.empty();
    bool hasUV = !uvs.empty();

    glm::vec3 mn(1e9f), mx(-1e9f);
    std::map<std::tuple<int, int, int>, unsigned int> unique;
    unsigned int vc = 0;

    for (const glm::ivec3& corner : faceCorners)
    {
        auto key = std::make_tuple(corner.x, corner.y, corner.z);
        auto found = unique.find(key);
        if (found == unique.end())
        {
            unsigned int id = vc++;
            unique[key] = id;

            glm::vec3 p = pos[corner.x - 1];
            glm::vec3 n = (hasN && corner.z > 0) ? nrm[corner.z - 1] : glm::vec3(0, 1, 0);
            glm::vec2 t = (hasUV && corner.y > 0) ? uvs[corner.y - 1] : glm::vec2(0);

            out.interleaved.push_back(p.x); out.interleaved.push_back(p.y); out.interleaved.push_back(p.z);
            out.interleaved.push_back(n.x); out.interleaved.push_back(n.y); out.interleaved.push_back(n.z);
            out.interleaved.push_back(t.x); out.interleaved.push_back(t.y);

            mn = glm::min(mn, p);
            mx = glm::max(mx, p);
            out.indices.push_back(id);
        }
        else
        {
            out.indices.push_back(found->second);
        }
    }

    out.center = (mn + mx) * 0.5f;
    out.minY = mn.y;
    glm::vec3 size = mx - mn;
    float maxExtent = glm::max(size.x, glm::max(size.y, size.z));
    out.invExtent = (maxExtent > 1e-4f) ? (1.0f / maxExtent) : 1.0f;
    return true;
}
