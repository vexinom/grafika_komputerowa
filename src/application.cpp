#include "application.h"
#include "skydome.h"

#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
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
    scene.reef.DrawDepth(*shaders["depthobject"], lightSpaceMatrix);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height);
}


void Application::drawFBO(float time)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height); 

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); 
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    glm::mat4 proj = scene.camera.GetProjectionMatrix();
    glm::mat4 viewMat = scene.camera.GetViewMatrix();

    // project the sun onto the screen for the volumetric light shafts
    glm::vec4 sunClip = proj * viewMat * glm::vec4(scene.camera.Position + glm::normalize(scene.sun.direction) * 2000.0f, 1.0f);
    glm::vec2 sunScreen(0.5f);
    float sunVisible = 0.0f;
    if (sunClip.w > 0.0f)
    {
        glm::vec3 ndc = glm::vec3(sunClip) / sunClip.w;
        sunScreen = glm::vec2(ndc.x, ndc.y) * 0.5f + 0.5f;
        sunVisible = 1.0f;
    }

    shaders["postprocess"]->Use();
    shaders["postprocess"]->SetInt("sceneColor", 0);
    shaders["postprocess"]->SetInt("sceneDepth", 1);
    shaders["postprocess"]->SetMat4("invViewProj", glm::inverse(proj * viewMat));
    shaders["postprocess"]->SetVec3("cameraPos", scene.camera.Position);
    shaders["postprocess"]->SetVec3("sunDirection", scene.sun.direction);
    shaders["postprocess"]->SetFloat("time", time);
    shaders["postprocess"]->SetFloat("fogDensity", scene.fogDensity);
    shaders["postprocess"]->SetVec2("sunScreenPos", sunScreen);
    shaders["postprocess"]->SetFloat("sunVisible", sunVisible);
    shaders["postprocess"]->SetFloat("base_water_level", scene.watermesh.waterLevel);

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

        scene.axolotl.Update(deltaTime);
        scene.particles.Update(deltaTime, scene.camera.Position, currentFrame, scene.current);

        glm::vec3 headlightPos = scene.axolotl.HeadlightPosition();
        glm::vec3 headlightColor = scene.headlightOn ? glm::vec3(1.6f, 1.5f, 1.2f) : glm::vec3(0.0f);
        scene.worldmesh.headlightPos = headlightPos;
        scene.worldmesh.headlightColor = headlightColor;

        ShadowPass();

        waterFrameBuffer.bindOceandepthFrameBuffer();
        glClearColor( 0.1f, 0.1f, 0.1f, 1.0f);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        glm::mat4 view = scene.camera.GetViewMatrix();
        glm::mat4 projection = scene.camera.GetProjectionMatrix();
        glm::mat4 viewProjection = projection * view;

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        shaders["worldmesh"]->Use();
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);

        scene.worldmesh.Draw(*shaders["worldmesh"], view, projection, scene.camera.Position, scene.sun.direction, lightSpaceMatrix, shadowMap);

        scene.tube.Draw(*shaders["tube"], view, projection, scene.sun.direction, scene.camera.Position);

        scene.monument.Draw(*shaders["object"], view, projection, scene.sun.direction, scene.camera.Position);

        scene.reef.Draw(*shaders["reef"], view, projection, scene.sun.direction, scene.camera.Position, lightSpaceMatrix, shadowMap, headlightPos, headlightColor);

        scene.axolotl.Draw(*shaders["axolotl"], view, projection, scene.sun.direction, scene.camera.Position, lightSpaceMatrix, shadowMap);

        if (useCubemap)
            scene.cubemap.Draw(*shaders["cubemap"], view, projection);
        else
            scene.skydome.Draw(*shaders["skydome"], viewProjection, scene.camera.Position, scene.sun.direction, currentFrame);


        glDepthMask(GL_TRUE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);

        // suspended particles (bubbles + marine snow), then the water surface
        scene.particles.Draw(*shaders["particle"], view, projection, (float)height);

        // particles restore their own GL state, so re-assert transparency for the water
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);

        scene.watermesh.Draw(*shaders["watermesh"], view, projection, scene.camera.Position, scene.sun.direction, scene.worldmesh.heightmapTexture, currentFrame);

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
        scene.camera.SetView(glm::vec3(1024.0f, 360.0f, 1844.0f), 0.0f, glm::radians(-35.0f));

    if(glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
    {
        glm::vec3 target = scene.axolotl.NearestTo(scene.camera.Position);
        scene.camera.LookAt(target + glm::vec3(24.0f, 28.0f, 24.0f), target);
    }

    // ---- Interaction 1: submarine / creature headlight on-off (L) ----
    if(glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
    {
        if(!headlightKeyDown)
        {
            scene.headlightOn = !scene.headlightOn;
            headlightKeyDown = true;
        }
    }
    else
    {
        headlightKeyDown = false;
    }

    // ---- Interaction 2: underwater visibility / fog density (5 clearer, 6 murkier) ----
    if(glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS)
        scene.fogDensity = glm::clamp(scene.fogDensity - deltaTime * 0.8f, 0.2f, 3.0f);
    if(glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS)
        scene.fogDensity = glm::clamp(scene.fogDensity + deltaTime * 0.8f, 0.2f, 3.0f);

    // ---- Interaction 3: water current strength (7 weaker, 8 stronger) ----
    if(glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS)
        scene.current *= glm::max(0.0f, 1.0f - deltaTime);
    if(glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS)
        scene.current += glm::normalize(glm::vec3(1.0f, 0.0f, 0.5f)) * deltaTime * 20.0f;

    // ---- Interaction 4: pause / resume creatures (Space) ----
    if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        if(!pauseKeyDown)
        {
            scene.axolotl.paused = !scene.axolotl.paused;
            pauseKeyDown = true;
        }
    }
    else
    {
        pauseKeyDown = false;
    }

    // ---- Interaction 5: creature cruise speed (Up / Down arrows) ----
    if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        scene.axolotl.speedScale = glm::clamp(scene.axolotl.speedScale + deltaTime, 0.2f, 4.0f);
    if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        scene.axolotl.speedScale = glm::clamp(scene.axolotl.speedScale - deltaTime, 0.2f, 4.0f);

    // ---- Interaction 6: aim + left-click to poke a creature (ray picking) ----
    if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        if(!pokeMouseDown)
        {
            glm::vec3 forward = scene.camera.Orientation * glm::vec3(0.0f, 0.0f, -1.0f);
            scene.axolotl.Poke(scene.camera.Position + forward * 120.0f);
            pokeMouseDown = true;
        }
    }
    else
    {
        pokeMouseDown = false;
    }
}

void Application::cleanUp()
{
    //waterFrameBuffer.cleanUp()
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
    shaders["depth"] = new Shader("shaders/depth_vertex.glsl", "shaders/depth_fragment.glsl");
    shaders["cubemap"] = new Shader("shaders/cubemap_vertex.glsl", "shaders/cubemap_fragment.glsl");
    shaders["tube"] = new Shader("shaders/tube_vertex.glsl", "shaders/tube_fragment.glsl");
    shaders["object"] = new Shader("shaders/object_vertex.glsl", "shaders/object_fragment.glsl");
    shaders["depthobject"] = new Shader("shaders/depth_object_vertex.glsl", "shaders/depth_fragment.glsl");
    shaders["axolotl"] = new Shader("shaders/axolotl_vertex.glsl", "shaders/axolotl_fragment.glsl");
    shaders["reef"] = new Shader("shaders/reef_vertex.glsl", "shaders/reef_fragment.glsl");
    shaders["seaweed"] = new Shader("shaders/seaweed_vertex.glsl", "shaders/seaweed_fragment.glsl");
    shaders["particle"] = new Shader("shaders/particle_vertex.glsl", "shaders/particle_fragment.glsl");
}
