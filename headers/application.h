#pragma once

#include <camera.h>
#include <scene.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <map>
#include <string>

#include "shader.h"
#include "waterFrameBuffers.h"

class Application
{
    public:
        GLFWwindow* window;
        Scene scene;
        std::map<std::string, Shader*> shaders;
        WaterFrameBuffers waterFrameBuffer;
        int height;
        int width;

        bool Init();
        bool Init_Shadow();
        void Run();
    
        void Input_Events();
        void drawFBO(float time);
        void ShadowPass();
        void cleanUp();
        void drawReflectionPreview();

        void openGLConfiguration();
        void openGLDisableMouse();
        void shadersInit();
        void drawUnderwaterObjects(glm::mat4 view, glm::mat4 projection, glm::vec4 clipPlane);
        void drawReflectionsReflaction();

        void UpdateViewport(int framebufferWidth, int framebufferHeight);
        void ToggleFullscreen();
        void UpdateThirdPersonCamera();
        glm::mat4 PlayerModelMatrix() const;
        static void FramebufferSizeCallback(GLFWwindow* window, int framebufferWidth, int framebufferHeight);

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

        unsigned int shadowFBO;
        unsigned int shadowMap;
        glm::mat4 lightSpaceMatrix;
        static const unsigned int SHADOW_RESOLUTION = 4096;

        bool useCubemap = false;
        bool cubemapKeyDown = false;
        bool drawRefRefl = true;

        // edge-trigger state for the new interactions
        bool headlightKeyDown = false;
        bool pauseKeyDown = false;
        bool pokeMouseDown = false;
        bool otterCycleKeyDown = false;
        bool otterStopKeyDown = false;        
        bool rKeyDown = false;

        bool teleportKeyDown = false;
        bool teleportToFish = false;   // false: nastepny H -> axolotl, true: nastepny H -> ryba

        bool fullscreenKeyDown = false;
        bool isFullscreen = false;

        bool thirdPersonMode = false;
        bool thirdPersonKeyDown = false;
        glm::vec3 playerPosition = glm::vec3(1024.0f, 42.0f, 1900.0f);

        int windowedPosX = 100;
        int windowedPosY = 100;
        int windowedWidth = 1280;
        int windowedHeight = 720;

        int m_reflFrameCounter = 0;
};
