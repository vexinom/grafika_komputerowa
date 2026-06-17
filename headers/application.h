#pragma once

#include <camera.h>
#include <scene.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <map>
#include <string>

#include "shader.h"

class Application
{
    public:
        GLFWwindow* window;
        Scene scene;
        std::map<std::string, Shader*> shaders;
        int height;
        int width;

        bool Init();
        bool Init_FBO();
        bool Init_Shadow();
        void Run();
        void Shutdown();
        void Input_Events();
        void UseFBO(float time);
        void ShadowPass();

        Application(int _width, int _height)
        {
            height = _height;
            width = _width;

            lastX = _width / 2.0;
            lastY = _height / 2.0;
        }

    private:
        double lastX = 400.0;
        double lastY = 300.0;
        bool firstMouse = true;

        float deltaTime = 0.0f;
        float lastFrame = 0.0f;

        unsigned int postProcessFBO;
        unsigned int colorBuffer;
        unsigned int depthBuffer;
        unsigned int quadVAO, quadVBO;

        unsigned int shadowFBO;
        unsigned int shadowMap;
        glm::mat4 lightSpaceMatrix;
        static const unsigned int SHADOW_RESOLUTION = 4096;

        bool useCubemap = true;
        bool cubemapKeyDown = false;

        // edge-trigger state for the new interactions
        bool headlightKeyDown = false;
        bool pauseKeyDown = false;
        bool pokeMouseDown = false;
};
