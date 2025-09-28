
#include "MainThreadRunner.h"
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include "Window.h"

static bool isGlfwActive = false;
static int windowCount = 0;
static std::mutex win_mtx;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// Must only be called from the main thread.
GLFWwindow* createGLFWWindow ( char* winTitle, bool isFullScreen, GLFWmonitor* monitor, Rect2d rect ) {

    GLFWwindow* window;

    if ( isFullScreen && !monitor ) {
        monitor = glfwGetPrimaryMonitor();
    }

    window = glfwCreateWindow(rect.width, rect.height, winTitle, monitor, NULL);

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

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    isGlfwActive = true;
    return true;

}

GLFWwindow* iSetFullScreen ( Window* window ) {

    return mainThreadRunner->scheduleAndWait ([window]() -> GLFWwindow* {

        GLFWwindow* glfwWindow = createGLFWWindow(
            window->getTitle(), window->isFullScreen(), window->getMonitor(), window->getDimensions() );

        if ( !glfwWindow ) {
            std::cout << "Failed to set full screen to: " << window->isFullScreen();
            return nullptr;
        }

        glfwSwapBuffers(glfwWindow);
        glfwMakeContextCurrent(glfwWindow);
        glfwSetWindowTitle(glfwWindow, window->getTitle()); 
        glfwSwapInterval(window->isVSyncEnabled() ? 1 : 0 );
        glfwSwapBuffers(glfwWindow);
    
        return glfwWindow;
        
    });

}

void Window::render ( float deltaTime ) {
    constexpr auto wBgColor = windowBackgroundColor;

    glClearColor(wBgColor.red, wBgColor.green, wBgColor.blue, wBgColor.alpha);
    
    glClear(GL_COLOR_BUFFER_BIT);
}

void Window::run ( ) {
    
    float invMaxFrameRate = 1.0F / this->maxFrameRate;
    bool isMtxLocked = false;
    float deltaTime = 0;

    std::chrono::duration<double> frameTime ( invMaxFrameRate );
    std::chrono::high_resolution_clock::time_point frameStart, frameEnd;
    std::chrono::high_resolution_clock::duration timeElapsed;

    glfwSwapBuffers(this->window);
    glfwMakeContextCurrent(this->window);
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

        if (this->frameRateChanged || this->vSynChanged || this->titleChanged || this->fullscreenChanged) {
            win_mtx.lock();
            isMtxLocked = true;
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

        if ( this->titleChanged ) {

            mainThreadRunner->scheduleAndWait ([this]() {
                glfwSetWindowTitle(this->window, this->winTitle);
            });

        }

        if ( this->fullscreenChanged ) {
            this->window = iSetFullScreen(this); // TODO: FIX THIS ( DOUBLE SCREENS + FIX DIMENSIONS LOGIC)

            if ( !this->window ) {
                return;
            }
        }

        if ( isMtxLocked ) {
            win_mtx.unlock();
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

    std::lock_guard<std::mutex> lock(win_mtx);

    if ( !this->thread || this->thread->get_id() != std::this_thread::get_id()) {
        this->shouldDestroy = true;
        return;
    }

    mainThreadRunner->scheduleAndWait ( [this]() -> void {

        if ( this->window ) {
            glfwDestroyWindow(window); 
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

    std::lock_guard<std::mutex> lock(win_mtx);

    if ( this->dimensions == dims ) {
        return;
    }

    this->dimensions = dims;

    if ( !this->window ) {
        return;
    }

    mainThreadRunner->scheduleAndWait ([this, &dims]() -> void {
        glfwSetWindowSize(this->window, dims.width, dims.height);
        glfwSetWindowPos(this->window, dims.xPos, dims.yPos);
    });

}

void Window::setMaxFrameRate ( float frameRate ) {
    std::lock_guard<std::mutex> lock(win_mtx);

    if ( this->maxFrameRate != frameRate ) {
        this->maxFrameRate = frameRate;
        this->frameRateChanged = true;
    }

}

void Window::setVSyncEnabled ( bool enabled ) {
    std::lock_guard<std::mutex> lock(win_mtx);

    if ( this->vSyncEnabled != enabled ) {
        this->vSyncEnabled = enabled;
        this->vSynChanged = true;
    }
}

void Window::setTitle ( char* newTitle ) {
    std::lock_guard<std::mutex> lock(win_mtx);

    if ( this->winTitle != newTitle ) {
        this->winTitle = newTitle;
        this->titleChanged = true;
    }
}

void Window::setFullScreen ( bool enabled ) {
    std::lock_guard<std::mutex> lock(win_mtx);

    if ( this->fullscreenEnabled != enabled ) {
        this->fullscreenEnabled = enabled;
        this->fullscreenChanged = true;
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