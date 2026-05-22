#include "application.h"

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

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        fprintf(stderr, "Failed to initialize GLAD\n");
        return false;
    }

    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);
    shader = new Shader("shaders/vertex.glsl", "shaders/fragment.glsl");

    glDisable(GL_CULL_FACE);
    scene.Init();

    return true;
}

void Application::Run()
{
    while(!glfwWindowShouldClose(window))
    {
        std::chrono::time_point<std::chrono::high_resolution_clock> start = std::chrono::high_resolution_clock::now();
        glfwPollEvents();
        Keyboard_Events();

        glClearColor( 0.1f, 0.1f, 0.1f, 1.0f);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shader->Use();

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = scene.camera.GetViewMatrix();
        glm::mat4 projection = scene.camera.GetProjectionMatrix();

        glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shader->ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shader->ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        glUniform1i(glGetUniformLocation(shader->ID, "terrainWidth"), scene.terrain.width);
        glUniform1i(glGetUniformLocation(shader->ID, "terrainHeight"), scene.terrain.height);

        scene.Render();

        


        glfwSwapBuffers(window);
        std::chrono::time_point<std::chrono::high_resolution_clock> stop = std::chrono::high_resolution_clock::now();
        std::chrono::microseconds duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
        double fps = 1e6 / static_cast<double>(duration.count());

        printf("FPS: %.2f\n", fps);
    }
}

void Application::Keyboard_Events()
{
    if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        scene.camera.Rotate(-0.02f, 0.0f);

    if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        scene.camera.Rotate(0.02f, 0.0f);

    if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        scene.camera.Rotate(0.0f, -0.02f);

    if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        scene.camera.Rotate(0.0f, 0.02f);

    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    scene.camera.MoveLocal(glm::vec3(0.0f, 3.5f, 0.0f));

    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        scene.camera.MoveLocal(glm::vec3(0.0f, -3.5f, 0.0f));

    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        scene.camera.MoveLocal(glm::vec3(-3.5f, 0.0f, 0.0f));

    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        scene.camera.MoveLocal(glm::vec3(3.5f, 0.0f, 0.0f));
}