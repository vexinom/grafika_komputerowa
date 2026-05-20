#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "camera.h"




int main()
{

    GLFWwindow* window = glfwCreateWindow(800, 600, "test", NULL, NULL);
    glfwMakeContextCurrent(window);

    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glEnable(GL_DEPTH_TEST);

    Camera cam;
   
    cam.MoveLocal(glm::vec3(0, 0, -1));
    printf("yy");

    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = cam.GetViewMatrix();
        glm::mat4 projection = cam.GetProjectionMatrix();


        glfwSwapBuffers(window);
    }
    glfwTerminate();
}