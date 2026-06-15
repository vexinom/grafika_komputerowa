#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <windows.h>

#include "application.h"
#include "config.h"


//for windows to force the use of stronger grahpic card
extern "C" {
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}


int main()
{
    Application app(config::WINDOW_WIDTH, config::WINDOW_HEIGHT);  // initialization of the entire app
    if (!app.Init())
    {
        glfwTerminate();
        return -1;
    }

    app.Run();
    glfwTerminate();
    return 0;
}