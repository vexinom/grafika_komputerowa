#include "Shader.h"

#include <glad/glad.h>
#include <fstream>
#include <sstream>
#include <iostream>

static std::string LoadFile(const char* path)
{
    std::ifstream file(path);
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
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

    unsigned int fShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fShader, 1, &fSrc, nullptr);
    glCompileShader(fShader);

    ID = glCreateProgram();
    glAttachShader(ID, vShader);
    glAttachShader(ID, fShader);
    glLinkProgram(ID);

    glDeleteShader(vShader);
    glDeleteShader(fShader);
}

void Shader::Use()
{
    glUseProgram(ID);
}