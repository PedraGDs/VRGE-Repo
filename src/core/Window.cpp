
#include "MainThreadRunner.h"
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include "Window.h"

static std::mutex global_win_mtx;
static bool isGlfwActive = false;
static int windowCount = 0;

constexpr uint8_t DIMENSION_CHANGED_FLAG  = 0b1;
constexpr uint8_t FULLSCREEN_CHANGED_FLAG = 0b10;
constexpr uint8_t FRAMERATE_CHANGED_FLAG  = 0b100;
constexpr uint8_t VSYNC_CHANGED_FLAG      = 0b1000;
constexpr uint8_t TITLE_CHANGED_FLAG      = 0b10000;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// Must only be called from the main thread.
GLFWwindow* createGLFWWindow ( char* winTitle, bool isFullScreen, GLFWmonitor* monitor, Rect2d& rect ) {

    const GLFWvidmode* mode = nullptr;
    GLFWwindow* window;

    if ( isFullScreen ) {
        if ( !monitor ) {
            monitor = glfwGetPrimaryMonitor();
        }

        if ( monitor ) {
            mode = glfwGetVideoMode (monitor);
        }

        if ( mode ) {
            rect = Rect2d(0, 0, mode->width, mode->height);
        }
    } 

    window = glfwCreateWindow(rect.width, rect.height, 
        winTitle, isFullScreen ? monitor : nullptr, nullptr);

    if ( window ) {
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    }
    
    return window;

}

// GLFW && Glad
// Must only be called from the main thread.
bool initGLFW () { 

    if ( !glfwInit() ) {
        std::cout << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    isGlfwActive = true;
    return true;

}

void Window::iSetFullScreen ( ) {

    glfwMakeContextCurrent(NULL);

    bool succeeded = mainThreadRunner->scheduleAndWait ([this]() -> bool {

        std::lock_guard<std::mutex> lock (global_win_mtx);

        if ( this->window ) {
            glfwDestroyWindow(this->window);
            windowCount--;
        }

        this->window = createGLFWWindow(
            this->winTitle, this->fullscreenEnabled, this->getMonitor(), this->dimensions );

        if ( !this->window ) {
            std::cout << "Failed to set full screen to: " << this->fullscreenEnabled << std::endl;
            this->destroy();
            return false;
        }

        GLFWwindow* oldContext = glfwGetCurrentContext(); // main thread context
        glfwMakeContextCurrent(this->window);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cout << "Failed to initialize GLAD" << std::endl;
            std::cout << "Failed to set full screen to: " << this->fullscreenEnabled << std::endl;
            this->destroy();

            glfwMakeContextCurrent(oldContext);
            return false;
        }

        glfwSwapBuffers(this->window);
        glfwSetWindowTitle(this->window, this->winTitle); 
        glfwSwapInterval(this->vSyncEnabled ? 1 : 0 );
        glfwSwapBuffers(this->window);
                
        glViewport(0, 0, this->dimensions.width, this->dimensions.height);
        windowCount++;

        glfwMakeContextCurrent(oldContext);
        return true;
    });

    if ( succeeded ) {
        glfwMakeContextCurrent(this->window);
    }

}

void Window::iSetFlag ( uint8_t flag, bool enabled ) {
    
    if ( enabled ) {
        this->changedFlags |= flag;
    } else {
        this->changedFlags &= ~flag;
    }

}

bool Window::isFlagEnabled ( uint8_t flag ) {
    return this->changedFlags & flag;
};

void Window::render ( float deltaTime ) {
    constexpr auto wBgColor = windowBackgroundColor;

    glClearColor(wBgColor.red, wBgColor.green, wBgColor.blue, wBgColor.alpha);
    
    glClear(GL_COLOR_BUFFER_BIT);
}

void Window::run ( ) {
    
    float invMaxFrameRate = 1.0F / this->maxFrameRate;
    float deltaTime = 0;

    std::chrono::duration<double> frameTime ( invMaxFrameRate );
    std::chrono::high_resolution_clock::time_point frameStart, frameEnd;
    std::chrono::high_resolution_clock::duration timeElapsed;

    glfwMakeContextCurrent(this->window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        this->destroy();
        return;
    }

    glfwSwapBuffers(this->window);
    glfwSwapInterval( this->vSyncEnabled ? 1 : 0 );
    glViewport(0, 0, this->dimensions.width, this->dimensions.height);

    while(!this->shouldDestroy && !glfwWindowShouldClose(this->window)) {

        frameStart = std::chrono::high_resolution_clock::now();

        this->render(deltaTime);
        glfwSwapBuffers(this->window);

        frameEnd = std::chrono::high_resolution_clock::now();
        timeElapsed = frameEnd - frameStart;

        if ( frameTime > timeElapsed ) {
            std::this_thread::sleep_for(frameTime - timeElapsed);
            deltaTime = invMaxFrameRate;
        } 
        
        else {
            deltaTime = std::chrono::duration<float>(timeElapsed).count();
        }

        if ( !this->changedFlags ) {
            continue;
        }

        this->localMtx.lock();
        
        if ( this->isFlagEnabled(FRAMERATE_CHANGED_FLAG)) {
            this->iSetFlag(FRAMERATE_CHANGED_FLAG, false);

            invMaxFrameRate = 1.0F / this->maxFrameRate;
            frameTime = std::chrono::duration<double>( invMaxFrameRate );
        }

        if ( this->isFlagEnabled(VSYNC_CHANGED_FLAG)) {
            this->iSetFlag(VSYNC_CHANGED_FLAG, false);

            glfwSwapInterval( this->vSyncEnabled ? 1 : 0 );
        }

        if ( this->isFlagEnabled(TITLE_CHANGED_FLAG)) {
            this->iSetFlag(TITLE_CHANGED_FLAG, false);

            mainThreadRunner->scheduleAndWait ([this]() {
                glfwSetWindowTitle(this->window, this->winTitle);
            });

        }

        if ( this->isFlagEnabled(DIMENSION_CHANGED_FLAG)) {
            this->iSetFlag(DIMENSION_CHANGED_FLAG, false);

            mainThreadRunner->scheduleAndWait ([this]() -> void {
                glfwSetWindowSize(this->window, this->dimensions.width, this->dimensions.height);
                glfwSetWindowPos(this->window, this->dimensions.xPos, this->dimensions.yPos);
            });

        }

        // must be the last if-statement besides the mtx
        if ( this->isFlagEnabled(FULLSCREEN_CHANGED_FLAG)) {
            this->iSetFlag(FULLSCREEN_CHANGED_FLAG, false);

            this->iSetFullScreen();
        }

        this->changedFlags = 0;
        this->localMtx.unlock();
        
    }

    this->destroy();

}

bool Window::init () { 

    std::lock_guard<std::mutex> lockA(this->localMtx);
    std::lock_guard<std::mutex> lockB(global_win_mtx);

    if ( this->window ) {
        std::cout << "Attempted to initialize a window twice" << std::endl;
        return false;
    }

    return mainThreadRunner->scheduleAndWait ( [this]() -> bool {

        if ( !isGlfwActive && !initGLFW() ) {
            return false;
        }

        this->window = createGLFWWindow(this->winTitle, this->fullscreenEnabled, NULL, this->dimensions);
        if (!this->window) {
            std::cout << "Failed to open GLFW window" << std::endl;
            return false;
        }

        this->thread = new std::thread(Window::run, this);
        windowCount++;

        return true;

    });
}

void Window::destroy () {

    std::lock_guard<std::mutex> lockA(this->localMtx);
    std::lock_guard<std::mutex> lockB(global_win_mtx);

    if ( (!this->thread) || (this->thread->get_id() != std::this_thread::get_id())) {
        this->shouldDestroy = true;
        return;
    }

    mainThreadRunner->scheduleAndWait ( [this]() -> void {

        if ( this->window ) {
            glfwDestroyWindow(this->window); 
            this->window = nullptr;
            windowCount--;
        }

        // Application shutdown.
        if ( !windowCount && isGlfwActive ) {  
            glfwTerminate();
            isGlfwActive = false;
        }

        std::free(this->winTitle);
        delete this->thread;

    });

}

void Window::setDimensions ( Rect2d dims ) {

    std::lock_guard<std::mutex> lock(this->localMtx);

    if ( this->dimensions != dims ) {
        this->iSetFlag(DIMENSION_CHANGED_FLAG, true);
        this->dimensions = dims;
    }

}

void Window::setMaxFrameRate ( float frameRate ) {

    std::lock_guard<std::mutex> lock(this->localMtx);

    if ( this->maxFrameRate != frameRate ) {
        this->iSetFlag(FRAMERATE_CHANGED_FLAG, true);
        this->maxFrameRate = frameRate;
    }

}

void Window::setVSyncEnabled ( bool enabled ) {
    std::lock_guard<std::mutex> lock(this->localMtx);

    if ( this->vSyncEnabled != enabled ) {
        this->iSetFlag(VSYNC_CHANGED_FLAG, true);
        this->vSyncEnabled = enabled;
    }
}

void Window::setTitle ( char* newTitle ) {
    std::lock_guard<std::mutex> lock(this->localMtx);

    if ( this->winTitle != newTitle ) {
        this->iSetFlag(TITLE_CHANGED_FLAG, true);
        this->winTitle = newTitle;
    }
}

void Window::setFullScreen ( bool enabled ) {
    std::lock_guard<std::mutex> lock(this->localMtx);

    if ( this->fullscreenEnabled != enabled ) {
        this->iSetFlag(FULLSCREEN_CHANGED_FLAG, true);
        this->fullscreenEnabled = enabled;
    }
}

Rect2d Window::getDimensions () {
    return this->dimensions;
}

bool Window::isFullScreen ( ) {
    return this->fullscreenEnabled;
}

bool Window::isVSyncEnabled ( ) {
    return this->vSyncEnabled;
}

float Window::getMaxFrameRate ( ) {
    return this->maxFrameRate;
}

std::thread* Window::getThread () {
    return this->thread;
}

char* Window::getTitle ( ) {
    return this->winTitle;
}

GLFWmonitor* Window::getMonitor ( ) {
    
    return mainThreadRunner->scheduleAndWait ([this]() -> GLFWmonitor* {

        GLFWmonitor* monitor = glfwGetWindowMonitor( this->window ); 

        if ( monitor ) {
            return monitor;
        }

        int monitorCount = 0;
        GLFWmonitor** monitors = glfwGetMonitors(&monitorCount); 

        GLFWmonitor* bestMonitor;
        int bestArea = -1;
        int area;

        Rect2d monitorRect;
        Rect2d winRect;

        glfwGetWindowPos(window, &winRect.xPos, &winRect.yPos);
        glfwGetWindowSize(window, &winRect.width, &winRect.height);

        for ( int i = 0; i < monitorCount; ++i ) {

            monitor = monitors[i];

            glfwGetMonitorWorkarea(monitor, &monitorRect.xPos, &monitorRect.yPos, 
                &monitorRect.width, &monitorRect.height);
        
            area = winRect.getIntersectionArea(monitorRect);

            if ( bestArea < area ) {
                bestMonitor = monitor;
                bestArea = area;
            }

        }

        return bestMonitor; // return value

    });

}