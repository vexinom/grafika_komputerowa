#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

#include "application.h"
#include "config.h"


#if defined(_WIN32) || defined(_WIN64)
extern "C" {
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}
#endif


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