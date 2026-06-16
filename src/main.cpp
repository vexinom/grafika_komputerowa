#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <windows.h>

#include "application.h"


//for windows to force the use of stronger grahpic card
extern "C" {
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}


int main()
{
    Application app(1280, 720);  // initialization of the entire app
    if (!app.Init())
    {
        glfwTerminate();
        return -1;
    }

    app.Run();
    glfwTerminate();
    return 0;
}