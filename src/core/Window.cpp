
#include "Window.h"
#include <chrono>

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// GLFW && Glad
bool initGLFW () { // this is can not be called in parallel

    if ( !glfwInit() ) {
        std::cout << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    isGlfwActive = true;
    return true;

}

void Window::render ( float deltaTime ) {
    glClearColor(0.07F, 0.13F, 0.17F, 1.0F);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Window::run ( ) {
    
    float invMaxFrameRate = 1.0F / this->maxFrameRate;
    float deltaTime = 0;

    std::chrono::duration<double> frameTime ( invMaxFrameRate );
    std::chrono::high_resolution_clock::time_point frameStart, frameEnd;
    std::chrono::high_resolution_clock::duration timeElapsed;

    glfwSwapBuffers(this->window);
    glfwMakeContextCurrent(this->window);
    glfwSwapInterval( this->vSyncEnabled ? 1 : 0 );

    while(!this->shouldDestroy && !glfwWindowShouldClose(this->window)) {

        frameStart = std::chrono::high_resolution_clock::now();

        this->render(deltaTime);
        glfwSwapBuffers(this->window);
        glfwPollEvents();

        frameEnd = std::chrono::high_resolution_clock::now();
        timeElapsed = frameEnd - frameStart;

        if ( frameTime > timeElapsed ) {
            std::this_thread::sleep_for(frameTime - timeElapsed);
            deltaTime = invMaxFrameRate;
        } 
        
        else {
            deltaTime = std::chrono::duration<float>(timeElapsed).count();
        }

        if ( this->frameRateChanged ) {
            invMaxFrameRate = 1.0F / this->maxFrameRate;
            frameTime = std::chrono::duration<double>( invMaxFrameRate );
            this->frameRateChanged = false;
        }   

        if ( this->vSynChanged ) {
            glfwSwapInterval( this->vSyncEnabled ? 1 : 0 );
            this->vSynChanged = false;
        }
        
    }

    this->destroy();
    
}

bool Window::init () { 

    std::lock_guard<std::mutex> lock(win_mtx);

    if ( this->window ) {
        std::cout << "Attempted to initialize a window twice" << std::endl;

        return false;
    }

    if ( !isGlfwActive && !initGLFW() ) {
        return false;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    this->window = glfwCreateWindow(800, 600, "Sexy Window Title", NULL, NULL);
    if (!this->window) {
        std::cout << "Failed to open GLFW window" << std::endl;
        return false;
    }

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    
    this->thread = new std::thread(Window::run, this);
    windowCount++;

    return true;
}

void Window::destroy () {

    std::lock_guard<std::mutex> lock(win_mtx);

    if ( !this->thread || this->thread->get_id() != std::this_thread::get_id()) {
        this->shouldDestroy = true;
        return;
    }

    if ( this->window ) {
        glfwDestroyWindow(window);
        windowCount--;
    }

    // Application shutdown.
    if ( !windowCount && isGlfwActive ) {  
        glfwTerminate();
        isGlfwActive = false;
    }

}

void Window::setDimensions ( Rect2d dims ) {

    std::lock_guard<std::mutex> lock(win_mtx);

    if ( this->dimensions == dims ) {
        return;
    }

    this->dimensions = dims;

    if ( this->window ) {
        glfwSetWindowSize(this->window, dims.width, dims.height);
        glfwSetWindowPos(this->window, dims.xPos, dims.yPos);
    }

}

Rect2d Window::getDimensions () {
    return this->dimensions;
}

std::thread* Window::getThread () {
    std::lock_guard<std::mutex> lock(win_mtx);
    return this->thread;
}

void Window::setMaxFrameRate ( float frameRate ) {
    this->maxFrameRate = frameRate;
    this->frameRateChanged = true;
}

float Window::getMaxFrameRate ( ) {
    return this->maxFrameRate;
}

void Window::setVSyncEnabled ( bool enabled ) {
    this->vSyncEnabled = enabled;
    this->vSynChanged = true;
}

bool Window::isVSyncEnabled ( ) {
    return this->vSyncEnabled;
}