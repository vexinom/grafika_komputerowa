#include "Shader.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <iostream>

static std::string LoadFile(const char* path)
{
    std::ifstream file(path);
    if (!file.is_open())
        std::cerr << "[Shader] ERROR: could not open file: " << path << std::endl;
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

// Prints the GLSL compile log for a shader stage; returns true on success.
static bool CheckCompile(unsigned int shader, const char* path)
{
    int ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        char log[2048];
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        std::cerr << "\n[Shader] COMPILE ERROR in " << path << ":\n" << log << std::endl;
    }
    return ok != 0;
}

Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
    std::string vCode = LoadFile(vertexPath);
    std::string fCode = LoadFile(fragmentPath);

    const char* vSrc = vCode.c_str();
    const char* fSrc = fCode.c_str();

    unsigned int vShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vShader, 1, &vSrc, nullptr);
    glCompileShader(vShader);
    CheckCompile(vShader, vertexPath);

    unsigned int fShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fShader, 1, &fSrc, nullptr);
    glCompileShader(fShader);
    CheckCompile(fShader, fragmentPath);

    ID = glCreateProgram();
    glAttachShader(ID, vShader);
    glAttachShader(ID, fShader);
    glLinkProgram(ID);

    int linked = 0;
    glGetProgramiv(ID, GL_LINK_STATUS, &linked);
    if (!linked)
    {
        char log[2048];
        glGetProgramInfoLog(ID, sizeof(log), nullptr, log);
        std::cerr << "\n[Shader] LINK ERROR (" << vertexPath << " + " << fragmentPath << "):\n" << log << std::endl;
    }

    glDeleteShader(vShader);
    glDeleteShader(fShader);
}


void Shader::Use()
{
    glUseProgram(ID);
}

void Shader::SetBool(const std::string& name, bool value)
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void Shader::SetInt(const std::string& name, int value)
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::SetFloat(const std::string& name, float value)
{
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}



void Shader::SetMat4(const std::string& name, const glm::mat4& value) {
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::SetVec2(const std::string& name, const glm::vec2& value)
{
    glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value));
}


void Shader::SetVec3(const std::string& name, const glm::vec3& value)
{
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}

void Shader::SetVec4(const std::string& name, const glm::vec4& value)
{
    glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}
