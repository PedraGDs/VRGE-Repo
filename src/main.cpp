#include <iostream>
#include <glad/glad.h>
#include <glfw/glfw3.h>
#include "core/MainThreadRunner.h"
#include "core/AppWindow.h"

MainThreadRunner* mainThreadRunner = nullptr;

int main(int argc, char** args) 
{
    mainThreadRunner = new MainThreadRunner();
    initGlfw();

    AppWindow window("Test Window");
    window.setMaxFrameRate(30);

    mainThreadRunner->addRepeating ([]() -> void { glfwPollEvents(); });

    if ( !window.init () ) {
        std::cout << "Failed to initialize window." << std::endl;
        return 1;
    }

    mainThreadRunner->start();

    // shutdown
    stopGlfw();
    return 0;
}
