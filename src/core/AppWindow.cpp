
#include <unordered_map>
#include "MainThreadRunner.h"
#include "AppWindow.h"

using highResClock = std::chrono::high_resolution_clock;

static std::unordered_map<GLFWwindow*, AppWindow*> windowMap{};
static std::mutex global_win_mtx;
static bool isGlfwActive = false;
static int windowCount = 0;

constexpr uint8_t POSITION_CHANGED_FLAG   = 0b1;
constexpr uint8_t SIZE_CHANGED_FLAG       = 0b10;
constexpr uint8_t FULLSCREEN_CHANGED_FLAG = 0b100;
constexpr uint8_t FRAMERATE_CHANGED_FLAG  = 0b1000;
constexpr uint8_t VSYNC_CHANGED_FLAG      = 0b10000;
constexpr uint8_t TITLE_CHANGED_FLAG      = 0b100000;
constexpr uint8_t ICON_CHANGED_FLAG       = 0b1000000;
constexpr uint8_t VISIBILITY_CHANGED_FLAG = 0b10000000;

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

        if ( this->visible ){
            glfwShowWindow(this->window);
        } 
        
        else {
            glfwHideWindow(this->window);
        }
        

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

AppWindow::~AppWindow ( ) {
    this->destroy();
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

        return bestMonitor; 

    });

}

void AppWindow::setFlag ( uint8_t flag, bool enabled ) {
    
    if ( enabled ) {
        this->changedFlags |= flag;
    } else {
        this->changedFlags &= ~flag;
    }

}

bool AppWindow::isFlagEnabled ( uint8_t flag ) {
    return this->changedFlags & flag;
};

void AppWindow::applyChanges ( ) {

    this->localMtx.lock();
        
    if ( this->isFlagEnabled(FRAMERATE_CHANGED_FLAG)) {
        this->setFlag(FRAMERATE_CHANGED_FLAG, false);

        this->frameTime = highResClock::duration(
            static_cast<highResClock::rep>(highResClock::period::den / this->maxFrameRate)
        );
    }

    if ( this->isFlagEnabled(VSYNC_CHANGED_FLAG)) {
        this->setFlag(VSYNC_CHANGED_FLAG, false);

        glfwSwapInterval( this->vSyncEnabled ? 1 : 0 );
    }

    if ( this->isFlagEnabled(TITLE_CHANGED_FLAG)) {
        this->setFlag(TITLE_CHANGED_FLAG, false);

        mainThreadRunner->scheduleAndWait<void> ([this]() -> void {
            glfwSetWindowTitle(this->window, this->winTitle);
        });
    }

    if ( this->isFlagEnabled(POSITION_CHANGED_FLAG) ) {
        this->setFlag(POSITION_CHANGED_FLAG, false);

        mainThreadRunner->scheduleAndWait<void> ([this]() -> void {
            glfwSetWindowPos(this->window, this->dimensions.xPos, this->dimensions.yPos);
        });
    }

    if ( this->isFlagEnabled(SIZE_CHANGED_FLAG) ) {
        this->setFlag(SIZE_CHANGED_FLAG, false);

        mainThreadRunner->scheduleAndWait<void> ([this]() -> void {
            glfwSetWindowSize(this->window, this->dimensions.width, this->dimensions.height);
        });
    }

    if ( this->isFlagEnabled(VISIBILITY_CHANGED_FLAG)) {
        this->setFlag(VISIBILITY_CHANGED_FLAG, false);
            
        mainThreadRunner->scheduleAndWait<void> ([this]() -> void {
            if ( this->visible ) {
                glfwShowWindow(this->window);
            } else {
                glfwHideWindow(this->window);
            }
        });
    }

    // must be the last if-statement besides the mtx
    if ( this->isFlagEnabled(FULLSCREEN_CHANGED_FLAG)) {
        this->setFlag(FULLSCREEN_CHANGED_FLAG, false);

        this->iSetFullScreen();
    }

    this->changedFlags = 0;
    this->localMtx.unlock();

}

void AppWindow::render ( float deltaTime ) {
    // constexpr auto wBgColor = windowBackgroundColor;
    // glClearColor(wBgColor.red, wBgColor.green, wBgColor.blue, wBgColor.alpha);
    // glClear(GL_COLOR_BUFFER_BIT);

    // std::cout << ( 1 / deltaTime ) << " FPS" << std::endl;
}

void AppWindow::run ( ) {

    this->localMtx.lock();

    float deltaTime = 0;

    this-> frameTime = highResClock::duration ( 
        static_cast<highResClock::rep>(highResClock::period::den / this->maxFrameRate));

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

        GLFWmonitor* monitor = this->getMonitor();
        GLFWwindow* oldContext = glfwGetCurrentContext();
        glfwMakeContextCurrent(this->window);

        if ( this->visible ) {
            glfwShowWindow ( this->window );
        } 
        
        else {
            glfwHideWindow ( this->window );
        }

        if ( !this->initializeCentered ) {
            glfwSetWindowSize(this->window, this->dimensions.width, this->dimensions.height);
            glfwSetWindowPos(this->window, this->dimensions.xPos, this->dimensions.yPos);
            return;
        } 

        if ( monitor ) {
            Rect2d monitorRect = getMonitorRect(monitor);
            Vector2i newPos{};
            
            std::cout << "Initialized window on: " << glfwGetMonitorName(monitor) << std::endl;
            newPos = monitorRect.getPos() + ((monitorRect.getSize() - this->dimensions.getSize()) / 2);
            
            this->dimensions.xPos = newPos.X;
            this->dimensions.yPos = newPos.Y;
        }

        std::cout << "Window Rect: " << this->dimensions << std::endl;
        
        glfwSetWindowSize(this->window, this->dimensions.width, this->dimensions.height);
        glfwSetWindowPos(this->window, this->dimensions.xPos, this->dimensions.yPos);

        glfwMakeContextCurrent(oldContext);

    });

    this->localMtx.unlock();

    while(!this->shouldDestroy && !glfwWindowShouldClose(this->window)) {

        if ( this->visible ) {

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

        }

        if ( this->changedFlags ) {
            this->applyChanges();
        }
        
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

    if ( this->dimensions != dims ) {
        std::lock_guard<std::mutex> lock(this->localMtx);

        this->setFlag(POSITION_CHANGED_FLAG, true);
        this->setFlag(SIZE_CHANGED_FLAG, true);
        this->dimensions = dims;
    }

}

void AppWindow::setSize ( int width, int height, bool isCallback ) {
    
    if ( this->dimensions.width == width && this->dimensions.height == height ) {
        return;
    }

    std::lock_guard<std::mutex> lock(this->localMtx);

    if ( isCallback ) {
        this->setFlag(SIZE_CHANGED_FLAG, false); // tells the run loop to ignore the change
        this->dimensions.height = height;
        this->dimensions.width = width;
    } 

    else {
        this->setFlag(SIZE_CHANGED_FLAG, true);
        this->dimensions.height = height;
        this->dimensions.width = width;
    }
}

void AppWindow::setPos ( int xPos, int yPos, bool isCallback ) {

    if ( this->dimensions.xPos == xPos && this->dimensions.yPos == yPos ) {
        return;
    }

    std::lock_guard<std::mutex> lock(this->localMtx);

    if ( isCallback ) {
        this->setFlag(POSITION_CHANGED_FLAG, false); // tells the run loop to ignore the change
        this->dimensions.xPos = xPos;
        this->dimensions.yPos = yPos;
    }

    else {
        this->setFlag(POSITION_CHANGED_FLAG, true);
        this->dimensions.xPos = xPos;
        this->dimensions.yPos = yPos;
    }

}

void AppWindow::setVisible ( bool enabled ) {

    if ( this->visible != enabled ) {
        std::lock_guard<std::mutex> lock(this->localMtx);

        this->setFlag(VISIBILITY_CHANGED_FLAG, true);
        this->visible = enabled;
    }

}

void AppWindow::setMaxFrameRate ( float frameRate ) {

    if ( this->maxFrameRate != frameRate ) {
        std::lock_guard<std::mutex> lock(this->localMtx);

        this->setFlag(FRAMERATE_CHANGED_FLAG, true);
        this->maxFrameRate = frameRate;
    }

}

void AppWindow::setVSyncEnabled ( bool enabled ) {

    if ( this->vSyncEnabled != enabled ) {
        std::lock_guard<std::mutex> lock(this->localMtx);

        this->setFlag(VSYNC_CHANGED_FLAG, true);
        this->vSyncEnabled = enabled;
    }
}

void AppWindow::setTitle ( const char* newTitle ) {

    if ( this->winTitle != newTitle ) {
        std::lock_guard<std::mutex> lock(this->localMtx);

        this->setFlag(TITLE_CHANGED_FLAG, true);
        this->winTitle = newTitle;
    }
}

void AppWindow::setFullScreen ( bool enabled ) {

    if ( this->fullscreenEnabled != enabled ) {
        std::lock_guard<std::mutex> lock(this->localMtx);
        
        this->setFlag(FULLSCREEN_CHANGED_FLAG, true);
        this->fullscreenEnabled = enabled;
    }
}

void AppWindow::setBufferSize ( int width, int height ) {

    if ( this->bufferSize.X != width && this->bufferSize.Y != height ) {
        std::lock_guard<std::mutex> lock(this->localMtx);

        this->bufferSize.X = width;
        this->bufferSize.Y = height;
    }

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

bool AppWindow::isVisible ( ) {
    return this->visible;
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
