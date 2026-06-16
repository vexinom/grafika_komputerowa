#include "application.h"
#include "skydome.h"

#include <iostream>
#include <glm/gtc/type_ptr.hpp>
<<<<<<< HEAD
#include <glm/gtc/matrix_transform.hpp>
=======
>>>>>>> c03a168ac1ee55ce4304085a785f225cd8fd36c5
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

<<<<<<< HEAD
=======
    //Initialization of shaders

>>>>>>> c03a168ac1ee55ce4304085a785f225cd8fd36c5
    shaders["worldmesh"] = new Shader("shaders/worldmesh_vertex.glsl", "shaders/worldmesh_fragment.glsl");
    shaders["watermesh"] = new Shader("shaders/watermesh_vertex.glsl", "shaders/watermesh_fragment.glsl");
    shaders["skydome"] = new Shader("shaders/skydome_vertex.glsl", "shaders/skydome_fragment.glsl");
    shaders["postprocess"] = new Shader("shaders/postprocess_vertex.glsl", "shaders/postprocess_fragment.glsl");
<<<<<<< HEAD
    shaders["depth"] = new Shader("shaders/depth_vertex.glsl", "shaders/depth_fragment.glsl");
    shaders["cubemap"] = new Shader("shaders/cubemap_vertex.glsl", "shaders/cubemap_fragment.glsl");
    shaders["tube"] = new Shader("shaders/tube_vertex.glsl", "shaders/tube_fragment.glsl");
    shaders["object"] = new Shader("shaders/object_vertex.glsl", "shaders/object_fragment.glsl");
    shaders["depthobject"] = new Shader("shaders/depth_object_vertex.glsl", "shaders/depth_fragment.glsl");
    shaders["axolotl"] = new Shader("shaders/axolotl_vertex.glsl", "shaders/axolotl_fragment.glsl");
=======
>>>>>>> c03a168ac1ee55ce4304085a785f225cd8fd36c5

    glDisable(GL_CULL_FACE);

    if(Init_FBO() == false)
    {
        return false;
    }
<<<<<<< HEAD
    if(Init_Shadow() == false)
    {
        return false;
    }
=======
>>>>>>> c03a168ac1ee55ce4304085a785f225cd8fd36c5
    scene.Init();

    return true;
}

<<<<<<< HEAD
bool Application::Init_Shadow()
{
    glGenFramebuffers(1, &shadowFBO);

    glGenTextures(1, &shadowMap);
    glBindTexture(GL_TEXTURE_2D, shadowMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_RESOLUTION, SHADOW_RESOLUTION, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float border[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);

    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    bool complete = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (!complete)
    {
        fprintf(stderr, "Failed to complete shadow framebuffer\n");
    }
    return complete;
}

void Application::ShadowPass()
{
    glm::vec3 lightDir = glm::normalize(scene.sun.direction);
    glm::vec3 center = scene.camera.Position;
    glm::vec3 up = glm::abs(lightDir.y) > 0.99f ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(0.0f, 1.0f, 0.0f);

    glm::mat4 lightView = glm::lookAt(center + lightDir * 500.0f, center, up);
    glm::mat4 lightProjection = glm::ortho(-400.0f, 400.0f, -400.0f, 400.0f, 1.0f, 1000.0f);
    lightSpaceMatrix = lightProjection * lightView;

    glViewport(0, 0, SHADOW_RESOLUTION, SHADOW_RESOLUTION);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    shaders["depth"]->Use();
    scene.worldmesh.DrawDepth(*shaders["depth"], lightSpaceMatrix);

    shaders["depthobject"]->Use();
    scene.monument.DrawDepth(*shaders["depthobject"], lightSpaceMatrix);
    scene.axolotl.DrawDepth(*shaders["depthobject"], lightSpaceMatrix);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height);
}

=======
>>>>>>> c03a168ac1ee55ce4304085a785f225cd8fd36c5
bool Application::Init_FBO()
{
    glGenFramebuffers(1, &postProcessFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, postProcessFBO);

    glGenTextures(1, &colorBuffer);
    glBindTexture(GL_TEXTURE_2D, colorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffer, 0);

    glGenTextures(1, &depthBuffer);
    glBindTexture(GL_TEXTURE_2D, depthBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthBuffer, 0);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        fprintf(stderr, "Failed to complete framebuffer\n");
        return false;
    }

    float quadVertices[] = {
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
        1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
        1.0f, -1.0f,  1.0f, 0.0f,
        1.0f,  1.0f,  1.0f, 1.0f
    };

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    return true;

}

void Application::UseFBO(float time)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f); 
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    shaders["postprocess"]->Use();
    shaders["postprocess"]->SetInt("sceneColor", 0);
    shaders["postprocess"]->SetInt("sceneDepth", 1);
    shaders["postprocess"]->SetMat4("invViewProj", glm::inverse(scene.camera.GetProjectionMatrix() * scene.camera.GetViewMatrix()));
    shaders["postprocess"]->SetVec3("cameraPos", scene.camera.Position);
    shaders["postprocess"]->SetVec3("sunDirection", scene.sun.direction);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorBuffer);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, depthBuffer);

    glBindVertexArray(quadVAO);
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

<<<<<<< HEAD
        scene.axolotl.Update(deltaTime);
        scene.worldmesh.headlightPos = scene.axolotl.HeadlightPosition();
        scene.worldmesh.headlightColor = glm::vec3(1.6f, 1.5f, 1.2f);

        ShadowPass();

        glBindFramebuffer(GL_FRAMEBUFFER, postProcessFBO);
        glClearColor( 0.1f, 0.1f, 0.1f, 1.0f);
=======
        glBindFramebuffer(GL_FRAMEBUFFER, postProcessFBO);
        glClearColor( 0.1f, 0.1f, 0.1f, 1.0f);                      //base background color
>>>>>>> c03a168ac1ee55ce4304085a785f225cd8fd36c5

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

<<<<<<< HEAD
        glm::mat4 view = scene.camera.GetViewMatrix();
        glm::mat4 projection = scene.camera.GetProjectionMatrix();
        glm::mat4 viewProjection = projection * view;

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
=======
        glm::mat4 view = scene.camera.GetViewMatrix();              //view thingies
        glm::mat4 projection = scene.camera.GetProjectionMatrix();
        glm::mat4 viewProjection = projection * view;

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);                  //mode of drawing poligons

        //Height map
>>>>>>> c03a168ac1ee55ce4304085a785f225cd8fd36c5

        shaders["worldmesh"]->Use();
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);

<<<<<<< HEAD
        scene.worldmesh.Draw(*shaders["worldmesh"], view, projection, scene.camera.Position, scene.sun.direction, lightSpaceMatrix, shadowMap);

        scene.tube.Draw(*shaders["tube"], view, projection, scene.sun.direction, scene.camera.Position);

        scene.monument.Draw(*shaders["object"], view, projection, scene.sun.direction, scene.camera.Position);

        scene.axolotl.Draw(*shaders["axolotl"], view, projection, scene.sun.direction, scene.camera.Position, lightSpaceMatrix, shadowMap);

        if (useCubemap)
            scene.cubemap.Draw(*shaders["cubemap"], view, projection);
        else
            scene.skydome.Draw(*shaders["skydome"], viewProjection, scene.camera.Position, scene.sun.direction, currentFrame);
=======
        scene.worldmesh.Draw(*shaders["worldmesh"], view, projection, scene.camera.Position, scene.sun.direction);

        //Skydome shader uniforms, must be rendered BEFORE water

        scene.skydome.Draw(*shaders["skydome"], viewProjection, scene.camera.Position, scene.sun.direction, currentFrame);
>>>>>>> c03a168ac1ee55ce4304085a785f225cd8fd36c5
        
        
        glDepthMask(GL_TRUE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);

<<<<<<< HEAD
        scene.watermesh.Draw(*shaders["watermesh"], view, projection, scene.camera.Position, scene.sun.direction, scene.worldmesh.heightmapTexture, currentFrame);

=======
        //Water shaders

        scene.watermesh.Draw(*shaders["watermesh"], view, projection, scene.camera.Position, scene.sun.direction, scene.worldmesh.heightmapTexture, currentFrame);

        //Other thingies

>>>>>>> c03a168ac1ee55ce4304085a785f225cd8fd36c5
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);

        UseFBO(currentFrame);


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
<<<<<<< HEAD

    if(glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
    {
        if(!cubemapKeyDown)
        {
            useCubemap = !useCubemap;
            cubemapKeyDown = true;
        }
    }
    else
    {
        cubemapKeyDown = false;
    }

    if(glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
        scene.worldmesh.roughness = glm::clamp(scene.worldmesh.roughness - deltaTime, 0.05f, 1.0f);
    if(glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
        scene.worldmesh.roughness = glm::clamp(scene.worldmesh.roughness + deltaTime, 0.05f, 1.0f);
    if(glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
        scene.worldmesh.metallic = glm::clamp(scene.worldmesh.metallic - deltaTime, 0.0f, 1.0f);
    if(glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
        scene.worldmesh.metallic = glm::clamp(scene.worldmesh.metallic + deltaTime, 0.0f, 1.0f);

    if(glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
        scene.camera.SetView(glm::vec3(1224.0f, 200.0f, 1180.0f), 0.0f, glm::radians(-18.0f));

    if(glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
    {
        glm::vec3 target = scene.axolotl.NearestTo(scene.camera.Position);
        scene.camera.LookAt(target + glm::vec3(24.0f, 28.0f, 24.0f), target);
    }
=======
>>>>>>> c03a168ac1ee55ce4304085a785f225cd8fd36c5
}