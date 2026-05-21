#pragma once

#include <camera.h>
#include <scene.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "shader.h"

class Application
{
    public:
        GLFWwindow* window;
        Scene scene;
        Shader* shader;
        int height;
        int width;

        bool Init();
        void Run();
        void Shutdown();
        void Keyboard_Events();

        Application(int _height, int _width)
        {
            height = _height;
            width = _width;
            shader = nullptr;
        }
};