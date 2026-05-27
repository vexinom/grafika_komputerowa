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
        Shader* waterShader;
        int height;
        int width;

        bool Init();                                    // initialization of application
        void Run();
        void Shutdown();
        void Input_Events();

        Application(int _width, int _height)
        {
            height = _height;
            width = _width;
            shader = nullptr;

            lastX = _width / 2.0;
            lastY = _height / 2.0;

        }

    private:
        double lastX = 400.0;  
        double lastY = 300.0;
        bool firstMouse = true;

        float deltaTime = 0.0f;                             // things for time
        float lastFrame = 0.0f;
};