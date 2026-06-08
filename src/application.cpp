#include "application.h"
#include "skydome.h"

#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <chrono>

bool Application::Init()
{
    
    if(!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return false;
    }
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    

    window = glfwCreateWindow(width, height, "OpenGL", NULL, NULL);

    if(!window)
    {
        fprintf(stderr, "Failed to create GLFW window\n");
        return false;
    }

    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (glfwRawMouseMotionSupported()) 
    {
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        fprintf(stderr, "Failed to initialize GLAD\n");
        return false;
    }

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        fprintf(stderr, "Failed to initialize GLAD\n");
        return false;
    }

    glfwSwapInterval(0);

    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);

    //Initialization of shaders

    shaders["terrain"] = new Shader("shaders/vertex.glsl", "shaders/fragment.glsl");
    shaders["water"] = new Shader("shaders/water_vertex.glsl", "shaders/water_fragment.glsl");
    shaders["skydome"] = new Shader("shaders/skydome_vertex.glsl", "shaders/skydome_fragment.glsl");

    glDisable(GL_CULL_FACE);
    scene.Init();

    return true;
}

void Application::Run()
{
    double lastTime = glfwGetTime();
    int frameCount = 0;

    while(!glfwWindowShouldClose(window))
    {

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;


        glfwPollEvents();
        Input_Events();

        scene.DailyCycle(currentFrame);

        glClearColor( 0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        //Height map shader uniforms

        shaders["terrain"]->Use();
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        

        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = scene.camera.GetViewMatrix();
        glm::mat4 projection = scene.camera.GetProjectionMatrix();
        glm::mat4 viewProjection = projection * view;

        glUniformMatrix4fv(glGetUniformLocation(shaders["terrain"]->ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shaders["terrain"]->ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaders["terrain"]->ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform1i(glGetUniformLocation(shaders["terrain"]->ID, "terrainWidth"), scene.terrain.width);
        glUniform1i(glGetUniformLocation(shaders["terrain"]->ID, "terrainHeight"), scene.terrain.height);

        unsigned int terrainSunLoc = glGetUniformLocation(shaders["terrain"]->ID, "sunDirection"); 
        glUniform3fv(terrainSunLoc, 1, &scene.sun.direction[0]);

        shaders["terrain"]->SetFloat("waterLevel", 80.0f);

        scene.terrain.Draw(viewProjection, scene.camera.Position, shaders["terrain"]->ID);

        //Skydome shader uniforms, must be rendered BEFORE water

        shaders["skydome"]->Use();
        glDepthMask(GL_FALSE); 

        unsigned int sunDirLoc = glGetUniformLocation(shaders["skydome"]->ID, "sunDirection");
        glUniform3fv(sunDirLoc, 1, &scene.sun.direction[0]);

        scene.skydome.Draw(viewProjection, scene.camera.Position, shaders["skydome"]->ID, currentFrame);
        glDepthMask(GL_TRUE);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);

        //Water shaders

        shaders["water"]->Use();
        shaders["water"]->SetMat4("model", glm::mat4(1.0f));
        shaders["water"]->SetMat4("view", scene.camera.GetViewMatrix());
        shaders["water"]->SetMat4("projection", scene.camera.GetProjectionMatrix());

        unsigned int waterSunLoc = glGetUniformLocation(shaders["water"]->ID, "sunDirection");
        glUniform3fv(waterSunLoc, 1, &scene.sun.direction[0]);

        unsigned int waterViewLoc = glGetUniformLocation(shaders["water"]->ID, "viewPos");
        glUniform3fv(waterViewLoc, 1, &scene.camera.Position[0]);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, scene.terrain.heightmapTexture);

        scene.water.Draw(viewProjection, scene.camera.Position, shaders["water"]->ID, currentFrame);

        //Other thingies

        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);

        glfwSwapBuffers(window);
        
        frameCount++;
        double currentTime = glfwGetTime();
        if (currentTime - lastTime >= 1.0)
        {
            char title[128];
            snprintf(title, sizeof(title), "OpenGL Terrain | FPS: %d", frameCount);
            glfwSetWindowTitle(window, title);
            
            frameCount = 0;
            lastTime = currentTime;
        }
    }
}

void Application::Input_Events()
{

    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    if(firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = static_cast<float>(xpos - lastX);
    float yoffset = static_cast<float>(lastY - ypos);

    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.001f; 
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    scene.camera.Rotate(xoffset, yoffset);

    float speedPerSecond = 250.0f; 
    float currentVelocity = speedPerSecond * deltaTime;


    if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        scene.camera.MoveLocal(glm::vec3(0.0f, currentVelocity, 0.0f));

    if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        scene.camera.MoveLocal(glm::vec3(0.0f, -currentVelocity, 0.0f));

    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        scene.camera.MoveLocal(glm::vec3(-currentVelocity, 0.0f, 0.0f));

    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        scene.camera.MoveLocal(glm::vec3(currentVelocity, 0.0f, 0.0f));

    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        scene.camera.MoveLocal(glm::vec3(0.0f, 0.0f, -currentVelocity));

    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        scene.camera.MoveLocal(glm::vec3(0.0f, 0.0f, currentVelocity));
}