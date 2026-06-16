#pragma once

#include <string>
#include <glm/glm.hpp>

class Shader
{
    public:
        unsigned int ID;

        Shader(const char* vertexPath, const char * fragmentPath);
        void Use();

        //Basic setters for types like float, int etc.
        void SetBool(const std::string& name, bool value);
        void SetFloat(const std::string& name, float value);
        void SetInt(const std::string& name, int value);

        //Setters for mat and vec
        void SetMat4(const std::string& name, const glm::mat4& mat);
        void SetVec2(const std::string& name, const glm::vec2& value);
        void SetVec3(const std::string& name, const glm::vec3& value);

};