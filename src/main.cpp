#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "application.h"




int main()
{
    Application app(720, 1280);
    if (!app.Init())
    {
        glfwTerminate();
        return -1;
    }

    app.Run();
    glfwTerminate();
    return 0;
}