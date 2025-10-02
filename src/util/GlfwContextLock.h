
#pragma once

#include "glad/glad.h"
#include "GLFW/glfw3.h"

// similar to std::lock_guard from mutex
struct GlfwContextLock {

    private:
        GLFWwindow* oldContext;
    
    public:
        GlfwContextLock ( GLFWwindow* newContext ) {
            this->oldContext = glfwGetCurrentContext ( );

            if ( this->oldContext != newContext ) {
                glfwMakeContextCurrent ( newContext );
            }
        }

        ~GlfwContextLock ( ) {
            glfwMakeContextCurrent ( this->oldContext);
        }

};
