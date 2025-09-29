
#include <unordered_map>
#include "MainThreadRunner.h"
#include "AppWindow.h"

using highResClock = std::chrono::high_resolution_clock;

static std::unordered_map<GLFWwindow*, AppWindow*> windowMap{};
static std::mutex global_win_mtx;
static bool isGlfwActive = false;
static int windowCount = 0;

constexpr uint8_t POSITION_CHANGED_FLAG      = 0b1;
constexpr uint8_t SIZE_CHANGED_FLAG          = 0b10;
constexpr uint8_t FULLSCREEN_CHANGED_FLAG    = 0b100;
constexpr uint8_t FRAMERATE_CHANGED_FLAG     = 0b1000;
constexpr uint8_t VSYNC_CHANGED_FLAG         = 0b10000;
constexpr uint8_t TITLE_CHANGED_FLAG         = 0b100000;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height); // update glad
    windowMap[window]->setBufferSize(width, height);
}

void window_size_callback(GLFWwindow* window, int width, int height) {
    windowMap[window]->setSize(width, height, true);
}

void window_pos_callback(GLFWwindow* window, int xPos, int yPos) {
    windowMap[window]->setPos(xPos, yPos, true);
}

inline void sleepUntil ( highResClock::time_point target ) {

    while ( highResClock::now() + std::chrono::microseconds(500) < target ) {
        std::this_thread::sleep_for ( std::chrono::microseconds(500) );
    }   

    while ( highResClock::now() < target ) { }

}

// Must only be called from the main thread.
bool initGlfw ( ) { 

    if ( isGlfwActive ) {
        return true;
    }

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

// Must only be called from the main thread.
void stopGlfw ( ) {

    if ( isGlfwActive) { 
        glfwTerminate ();
        isGlfwActive = false;
    }

}

// Must only be called from the main thread.
GLFWwindow* createGLFWWindow ( const char* winTitle, bool isFullScreen, 
    GLFWmonitor* monitor, Rect2d& rect ) {

    const GLFWvidmode* mode = nullptr;
    GLFWwindow* window, *oldContext;

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
        glfwSetWindowSizeCallback(window, window_size_callback);
        glfwSetWindowPosCallback(window, window_pos_callback);
    }
    
    return window;

}

void AppWindow::iSetFullScreen ( ) {

    glfwMakeContextCurrent(NULL);

    bool succeeded = mainThreadRunner->scheduleAndWait<bool> ([this]() -> bool {

        std::lock_guard<std::mutex> lock (global_win_mtx);

        if ( this->window ) {
            glfwDestroyWindow(this->window);
            windowMap.erase(this->window);
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
        windowMap[this->window] = this;

        return true;
    });

    if ( succeeded ) {
        glfwMakeContextCurrent(this->window);
    }

}

inline Rect2d getMonitorRect ( GLFWmonitor* monitor ) {
    
    Rect2d rect{};

    glfwGetMonitorWorkarea(monitor, &rect.xPos, &rect.yPos, &rect.width, &rect.height);

    return rect;

}

GLFWmonitor* AppWindow::getMonitor ( ) {
    
    return mainThreadRunner->scheduleAndWait<GLFWmonitor*> ([this]() -> GLFWmonitor* {

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
            monitorRect = getMonitorRect(monitor);

            area = winRect.getIntersectionArea(monitorRect);

            if ( bestArea < area ) {
                bestMonitor = monitor;
                bestArea = area;
            }

        }

        return bestMonitor; // return value

    });

}

void AppWindow::iSetFlag ( uint8_t flag, bool enabled ) {
    
    if ( enabled ) {
        this->changedFlags |= flag;
    } else {
        this->changedFlags &= ~flag;
    }

}

bool AppWindow::iIsFlagEnabled ( uint8_t flag ) {
    return this->changedFlags & flag;
};

AppWindow::~AppWindow ( ) {
    this->destroy();
}

void AppWindow::render ( float deltaTime ) {
    // constexpr auto wBgColor = windowBackgroundColor;
    // glClearColor(wBgColor.red, wBgColor.green, wBgColor.blue, wBgColor.alpha);
    // glClear(GL_COLOR_BUFFER_BIT);

    std::cout << ( 1 / deltaTime ) << " FPS" << std::endl;
}

void AppWindow::run ( ) {

    this->localMtx.lock();

    float deltaTime = 0;

    highResClock::duration frameTime( 
        static_cast<highResClock::rep>(
            highResClock::period::den / this->maxFrameRate
        )
    );

    highResClock::time_point frameStart, frameEnd;
    highResClock::duration timeElapsed;

    glfwMakeContextCurrent(this->window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        this->destroy();
        return;
    }

    windowMap[this->window] = this;
    glfwSwapInterval( this->vSyncEnabled ? 1 : 0 );

    mainThreadRunner->scheduleAndWait<void>([this]() -> void {

        if ( !this->initializeCentered ) {
            glfwSetWindowSize(this->window, this->dimensions.width, this->dimensions.height);
            glfwSetWindowPos(this->window, this->dimensions.xPos, this->dimensions.yPos);
        } 

        GLFWmonitor* monitor = this->getMonitor();

        if ( monitor ) {
            Rect2d monitorRect = getMonitorRect(monitor);
            Vector2i newPos{};

            newPos = monitorRect.getPos() + ((monitorRect.getSize() + this->dimensions.getSize()) / 2);
            
            this->dimensions.xPos = newPos.X;
            this->dimensions.yPos = newPos.Y;
        }

        glfwSetWindowSize(this->window, this->dimensions.width, this->dimensions.height);
        glfwSetWindowPos(this->window, this->dimensions.xPos, this->dimensions.yPos);
        
    });

    this->localMtx.unlock();

    while(!this->shouldDestroy && !glfwWindowShouldClose(this->window)) {

        frameStart = highResClock::now();

        this->render(deltaTime);
        glfwSwapBuffers(this->window);

        frameEnd = highResClock::now();
        timeElapsed = frameEnd - frameStart;

        if ( frameTime > timeElapsed ) {
            sleepUntil ( frameStart + frameTime );

            frameEnd = highResClock::now();
            timeElapsed = frameEnd - frameStart;
        } 
        
        deltaTime = std::chrono::duration<float>(timeElapsed).count();

        if ( !this->changedFlags ) {
            continue;
        }

        this->localMtx.lock();
        
        if ( this->iIsFlagEnabled(FRAMERATE_CHANGED_FLAG)) {
            this->iSetFlag(FRAMERATE_CHANGED_FLAG, false);

            frameTime = highResClock::duration(
                static_cast<highResClock::rep>(
                    highResClock::period::den / this->maxFrameRate
                )
            );
        }

        if ( this->iIsFlagEnabled(VSYNC_CHANGED_FLAG)) {
            this->iSetFlag(VSYNC_CHANGED_FLAG, false);

            glfwSwapInterval( this->vSyncEnabled ? 1 : 0 );
        }

        if ( this->iIsFlagEnabled(TITLE_CHANGED_FLAG)) {
            this->iSetFlag(TITLE_CHANGED_FLAG, false);

            mainThreadRunner->scheduleAndWait<void> ([this]() -> void {
                glfwSetWindowTitle(this->window, this->winTitle);
            });

        }

        if ( this->iIsFlagEnabled(POSITION_CHANGED_FLAG) ) {
            this->iSetFlag(POSITION_CHANGED_FLAG, false);

            mainThreadRunner->scheduleAndWait<void> ([this]() -> void {
                glfwSetWindowPos(this->window, this->dimensions.xPos, this->dimensions.yPos);
            });
        }

        if ( this->iIsFlagEnabled(SIZE_CHANGED_FLAG) ) {
            this->iSetFlag(SIZE_CHANGED_FLAG, false);

            mainThreadRunner->scheduleAndWait<void> ([this]() -> void {
                glfwSetWindowSize(this->window, this->dimensions.width, this->dimensions.height);
            });
        }

        // must be the last if-statement besides the mtx
        if ( this->iIsFlagEnabled(FULLSCREEN_CHANGED_FLAG)) {
            this->iSetFlag(FULLSCREEN_CHANGED_FLAG, false);

            this->iSetFullScreen();
        }

        this->changedFlags = 0;
        this->localMtx.unlock();
        
    }

    this->isActive = false;
    this->destroy();

}

bool AppWindow::init ( ) { 

    std::lock_guard<std::mutex> lockA(this->localMtx);
    std::lock_guard<std::mutex> lockB(global_win_mtx);

    if ( this->isDestroyed ) {
        std::cout << "Attempted to initialized a destroyed window" << std::endl;
        return false;
    }

    if ( this->isActive ) {
        std::cout << "Attempted to initialize a window twice" << std::endl;
        return false;
    }

    std::cout << "Initializing window: " << this->winTitle << std::endl;
    this->isActive = true;

    return mainThreadRunner->scheduleAndWait<bool> ( [this]() -> bool {

        if ( !isGlfwActive && !initGlfw() ) {
            std::cout << "Failed to initialize GLFW" << std::endl;
            return false;
        }

        this->window = createGLFWWindow(this->winTitle, this->fullscreenEnabled, NULL, this->dimensions);
        if (!this->window) {
            std::cout << "Failed to open GLFW window" << std::endl;
            return false;
        }

        this->thread = new std::thread(AppWindow::run, this);
        mainThreadRunner->addChild(this->thread);
        
        windowCount++;
        return true;

    });
}

void AppWindow::destroy () {

    std::lock_guard<std::mutex> lockA(this->localMtx);
    std::lock_guard<std::mutex> lockB(global_win_mtx);

    if ( this->isDestroyed ) {
        return;
    }

    if ( (!this->thread) || (this->thread->get_id() != std::this_thread::get_id())) {
        this->shouldDestroy = true;
        return;
    }

    mainThreadRunner->scheduleAndWait<void> ( [this]() -> void {

        if ( this->window ) {
            glfwDestroyWindow(this->window); 
            this->window = nullptr;
            windowCount--;
        }

        this->isDestroyed = true;
        windowMap.erase(this->window);

        // Application shutdown.
        if ( !windowCount && isGlfwActive ) {  
            mainThreadRunner->stop();
        } 
        
        else {
            mainThreadRunner->removeChild(this->thread);
            delete this->thread;
        }


    });

}

void AppWindow::setDimensions ( Rect2d dims ) {

    std::lock_guard<std::mutex> lock(this->localMtx);

    if ( this->dimensions != dims ) {
        this->iSetFlag(POSITION_CHANGED_FLAG, true);
        this->iSetFlag(SIZE_CHANGED_FLAG, true);
        this->dimensions = dims;
    }

}

void AppWindow::setSize ( int width, int height, bool isCallback ) {
    
    std::lock_guard<std::mutex> lock(this->localMtx);

    if ( this->dimensions.width == width && this->dimensions.height == height ) {
        return;
    }

    if ( isCallback ) {
        this->iSetFlag(SIZE_CHANGED_FLAG, false); // tells the run loop to ignore the change
        this->dimensions.height = height;
        this->dimensions.width = width;
    } 

    else {
        this->iSetFlag(SIZE_CHANGED_FLAG, true);
        this->dimensions.height = height;
        this->dimensions.width = width;
    }
}

void AppWindow::setPos ( int xPos, int yPos, bool isCallback ) {

    std::lock_guard<std::mutex> lock(this->localMtx);

    if ( this->dimensions.xPos == xPos && this->dimensions.yPos == yPos ) {
        return;
    }

    if ( isCallback ) {
        this->iSetFlag(POSITION_CHANGED_FLAG, false); // tells the run loop to ignore the change
        this->dimensions.xPos = xPos;
        this->dimensions.yPos = yPos;
    }

    else {
        this->iSetFlag(POSITION_CHANGED_FLAG, true);
        this->dimensions.xPos = xPos;
        this->dimensions.yPos = yPos;
    }

}

void AppWindow::setMaxFrameRate ( float frameRate ) {

    std::lock_guard<std::mutex> lock(this->localMtx);

    if ( this->maxFrameRate != frameRate ) {
        this->iSetFlag(FRAMERATE_CHANGED_FLAG, true);
        this->maxFrameRate = frameRate;
    }

}

void AppWindow::setVSyncEnabled ( bool enabled ) {
    std::lock_guard<std::mutex> lock(this->localMtx);

    if ( this->vSyncEnabled != enabled ) {
        this->iSetFlag(VSYNC_CHANGED_FLAG, true);
        this->vSyncEnabled = enabled;
    }
}

void AppWindow::setTitle ( const char* newTitle ) {
    std::lock_guard<std::mutex> lock(this->localMtx);

    if ( this->winTitle != newTitle ) {
        this->iSetFlag(TITLE_CHANGED_FLAG, true);
        this->winTitle = newTitle;
    }
}

void AppWindow::setFullScreen ( bool enabled ) {
    std::lock_guard<std::mutex> lock(this->localMtx);

    if ( this->fullscreenEnabled != enabled ) {
        this->iSetFlag(FULLSCREEN_CHANGED_FLAG, true);
        this->fullscreenEnabled = enabled;
    }
}

void AppWindow::setBufferSize ( int width, int height ) {
    std::lock_guard<std::mutex> lock(this->localMtx);

    this->bufferSize.X = width;
    this->bufferSize.Y = height;
}

Vector2i AppWindow::getBufferSize () {
    return this->bufferSize;
}

Rect2d AppWindow::getDimensions () {
    return this->dimensions;
}

Vector2i AppWindow::getSize () {
    return Vector2i(this->dimensions.width, this->dimensions.height);
}

Vector2i AppWindow::getPos () {
    return Vector2i(this->dimensions.xPos, this->dimensions.yPos);
}

bool AppWindow::isFullScreen ( ) {
    return this->fullscreenEnabled;
}

bool AppWindow::isVSyncEnabled ( ) {
    return this->vSyncEnabled;
}

float AppWindow::getMaxFrameRate ( ) {
    return this->maxFrameRate;
}

std::thread* AppWindow::getThread () {
    return this->thread;
}

const char* AppWindow::getTitle ( ) {
    return this->winTitle;
}
