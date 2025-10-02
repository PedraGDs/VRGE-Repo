#include <iostream>
#include <glad/glad.h>
#include <glfw/glfw3.h>
#include "core/MainThreadRunner.h"
#include "input/Keybindings.h"
#include "core/AppWindow.h"

MainThreadRunner* mainThreadRunner = nullptr;

int main(int argc, char** args) 
{
    mainThreadRunner = new MainThreadRunner();
    initGlfw();

    registerKeyBinds();
    mainThreadRunner->addRepeating ([]() -> void { glfwPollEvents(); });

    AppWindow window("Test Window");
    window.setFullScreen(true);

    if ( !window.init () ) {
        std::cout << "Failed to initialize window." << std::endl;
        return 1;
    }

    mainThreadRunner->start();
    
    stopGlfw();
    return 0;
}
