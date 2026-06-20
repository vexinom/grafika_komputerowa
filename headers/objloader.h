#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>

struct ObjMesh
{
    std::vector<float>        interleaved;
    std::vector<unsigned int> indices;
    glm::vec3 center    = glm::vec3(0.0f);  
    float     minY      = 0.0f;             
    float     invExtent = 1.0f;             
};

bool LoadObj(const std::string& path, ObjMesh& out);

unsigned int LoadTexture(const std::string& path, bool flipV = true);

unsigned int WhiteTexture();
