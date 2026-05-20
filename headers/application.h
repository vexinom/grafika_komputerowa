#pragma once

#include <GLFW/glfw3.h>

class Application
{
    public:
        GLFWwindow* window;
        int height;
        int width;

        Application(int _height, int _width)
        {
            height = _height;
            width = _width;
        }

        int Init();
        void Run();
        void Shutdown();
};