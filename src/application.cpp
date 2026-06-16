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
    
    openGLConfiguration();
   

    window = glfwCreateWindow(width, height, "OpenGL", NULL, NULL);

    if(!window)
    {
        fprintf(stderr, "Failed to create GLFW window\n");
        return false;
    }

    glfwMakeContextCurrent(window);
    openGLDisableMouse();
    

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

    shadersInit();

    glDisable(GL_CULL_FACE);

    waterFrameBuffer.init();
    scene.Init();

    return true;
}

void Application::drawFBO(float time)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height); 

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); 
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    shaders["postprocess"]->Use();
    shaders["postprocess"]->SetInt("sceneColor", 0);
    shaders["postprocess"]->SetInt("sceneDepth", 1);
    shaders["postprocess"]->SetMat4("invViewProj", glm::inverse(scene.camera.GetProjectionMatrix() * scene.camera.GetViewMatrix()));
    shaders["postprocess"]->SetVec3("cameraPos", scene.camera.Position);
    shaders["postprocess"]->SetVec3("sunDirection", scene.sun.direction);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, waterFrameBuffer.oceandepthColorBuffer);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, waterFrameBuffer.oceandepthDepthTexture);

    glBindVertexArray(waterFrameBuffer.quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
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

        waterFrameBuffer.bindOceandepthFrameBuffer();
        glClearColor( 0.1f, 0.1f, 0.1f, 1.0f);                      //base background color

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        glm::mat4 view = scene.camera.GetViewMatrix();              //view thingies
        glm::mat4 projection = scene.camera.GetProjectionMatrix();
        glm::mat4 viewProjection = projection * view;

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);                  //mode of drawing poligons

        //Height map

        shaders["worldmesh"]->Use();
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);

        scene.worldmesh.Draw(*shaders["worldmesh"], view, projection, scene.camera.Position, scene.sun.direction);

        //Skydome shader uniforms, must be rendered BEFORE water

        scene.skydome.Draw(*shaders["skydome"], viewProjection, scene.camera.Position, scene.sun.direction, currentFrame);
        
        
        glDepthMask(GL_TRUE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);

        //Water shaders

        scene.watermesh.Draw(*shaders["watermesh"], view, projection, scene.camera.Position, scene.sun.direction, scene.worldmesh.heightmapTexture, currentFrame);

        //Other thingies

        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);

        drawFBO(currentFrame);


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


void Application::openGLConfiguration()
{
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

void Application::openGLDisableMouse()
{
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (glfwRawMouseMotionSupported()) 
    {
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }
}

void Application::shadersInit()
{
    shaders["worldmesh"] = new Shader("shaders/worldmesh_vertex.glsl", "shaders/worldmesh_fragment.glsl");
    shaders["watermesh"] = new Shader("shaders/watermesh_vertex.glsl", "shaders/watermesh_fragment.glsl");
    shaders["skydome"] = new Shader("shaders/skydome_vertex.glsl", "shaders/skydome_fragment.glsl");
    shaders["postprocess"] = new Shader("shaders/postprocess_vertex.glsl", "shaders/postprocess_fragment.glsl");
}