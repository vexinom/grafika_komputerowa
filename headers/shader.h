#pragma once

#include <string>
#include <glm/glm.hpp>

class Shader
{
    public:
        unsigned int ID;

        Shader(const char* vertexPath, const char * fragmentPath);
        void Use();
        void SetMat4(const std::string& name, const glm::mat4& mat);
        void SetFloat(const std::string& name, float value);

};