
#include <unordered_map>
#include "util/GlfwContextLock.h"
#include "input/InputHandler.h" // key_callback
#include "MainThreadRunner.h"
#include "util/TimeUtil.h"
#include "util/detect.h"
#include "AppWindow.h"
#include <stdlib.h>

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

AppWindow* getAppWindow ( GLFWwindow* window ) {
    return windowMap[window];
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    getAppWindow(window)->setBufferSize(width, height);
}

void window_size_callback(GLFWwindow* window, int width, int height) {
    getAppWindow(window)->setSize(width, height, true);
}

void window_pos_callback(GLFWwindow* window, int xPos, int yPos) {
    getAppWindow(window)->setPos(xPos, yPos, true);
}

// Must only be called from the main thread.
void toggle_callbacks ( GLFWwindow* window, bool enabled ) {

    if ( enabled ) {
        glfwSetFramebufferSizeCallback (window, framebuffer_size_callback);
        glfwSetWindowSizeCallback (window, window_size_callback);
        glfwSetWindowPosCallback (window, window_pos_callback);
        glfwSetKeyCallback (window, key_callback); // InputHandler
    } else {
        glfwSetFramebufferSizeCallback (window, NULL);
        glfwSetWindowSizeCallback (window, NULL);
        glfwSetWindowPosCallback (window, NULL);
        glfwSetKeyCallback (window, NULL);
    }

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
    if ( isGlfwActive ) { 
        glfwTerminate ();
        isGlfwActive = false;
    }
}

bool isValidMonitorAddress ( GLFWmonitor* monitor ) {
    #if ((OPERATING_SYSTEM == OS_WINDOWS) || (OPERATING_SYSTEM == OS_CYGWIN))
        constexpr uintptr_t INVALID_MONITOR_HANDLE = 0xFEEEFEEEFEEEFEEEULL;
        return monitor != nullptr && reinterpret_cast<uint64_t>(monitor) != INVALID_MONITOR_HANDLE;
    #else
        return monitor != nullptr;
    #endif
}

// Must only be called from the Main Thread
inline Rect2d getMonitorWorkRect ( GLFWmonitor* monitor ) {
    Rect2d rect{};

    if ( isValidMonitorAddress(monitor) ) {
        glfwGetMonitorWorkarea(monitor, &rect.xPos, &rect.yPos, &rect.width, &rect.height);
    }


    return rect;
}

// Must only be called from the main thread.
GLFWwindow* createGlfwWindow ( const char* winTitle, bool &isFullScreen, 
    GLFWmonitor* monitor, Rect2d& rect ) {

    const GLFWvidmode* mode;
    GLFWwindow* window;

    if ( isFullScreen ) {

        if ( !isValidMonitorAddress(monitor) ){
            monitor = glfwGetPrimaryMonitor();
        }

        if ( isValidMonitorAddress(monitor) ) {
            mode = glfwGetVideoMode ( monitor );
        }

        if ( mode ) {
            
            rect.width = mode->width;
            rect.height = mode->height;

            glfwGetMonitorPos ( monitor, &rect.xPos, &rect.yPos );

        } else {
            std::cout << "Failed to enable fullscreen" << std::endl;
            isFullScreen = false;
        }
        
    }
    
    window = glfwCreateWindow(rect.width, rect.height, winTitle, isFullScreen ? monitor : NULL, NULL);

    if ( window ) {
        toggle_callbacks ( window, true );
    }
    
    return window;

}

GLFWmonitor* AppWindow::getMonitor ( ) {
    
    return mainThreadRunner->scheduleAndWait<GLFWmonitor*> ([this]() -> GLFWmonitor* {

        GLFWmonitor* monitor;

        if ( !this->window ) {
            monitor = glfwGetPrimaryMonitor();
            return isValidMonitorAddress(monitor) ? monitor : NULL;
        }

        monitor = glfwGetWindowMonitor( this->window ); 

        if ( isValidMonitorAddress(monitor) ) {
            return monitor;
        }

        int monitorCount = 0;
        GLFWmonitor** monitors = glfwGetMonitors(&monitorCount); 

        GLFWmonitor* bestMonitor = NULL;
        int bestArea = -1;

        Rect2d monitorRect;
        int area;

        for ( int i = 0; i < monitorCount; ++i ) {

            monitor = monitors[i];

            if ( !isValidMonitorAddress(monitor)) {
                continue;
            }

            monitorRect = getMonitorWorkRect(monitor);
            area = this->dimensions.getIntersectionArea(monitorRect);

            if ( bestArea < area ) {
                bestMonitor = monitor;
                bestArea = area;
            }

        }

        return bestMonitor; 

    });

}

MonitorData* AppWindow::getMonitorData ( ) {

    GLFWmonitor* monitor = this->getMonitor();

    if ( !monitor ) {
        return nullptr;
    }

    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    if ( !mode ) {
        return nullptr;
    }

    MonitorData* data = new MonitorData();
    glfwGetMonitorPos ( monitor, &data->xPos, &data->yPos );    

    data->handle      = monitor;
    data->width       = mode->width;
    data->height      = mode->height;
    data->redBits     = mode->redBits;
    data->greenBits   = mode->greenBits;
    data->blueBits    = mode->blueBits;
    data->refreshRate = mode->refreshRate;

    return data;

}

// Must only be called from the main thread
bool AppWindow::initWindow ( ) {

    GLFWmonitor* monitor;
    GlfwContextLock lock ( this->window );

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        this->destroy();
        return false;
    }

    if ( this->initializeCentered && (monitor = this->getMonitor()) ) { 
        Rect2d monitorRect = getMonitorWorkRect(monitor);

        Vector2i newPos = monitorRect.getPos();
        newPos += ((monitorRect.getSize() - this->dimensions.getSize()) / 2); // size offset
    
        if ( fullscreenEnabled ) {
            this->oldDimensions.setPos(newPos);
        } else {
            this->dimensions.setPos(newPos);
        }
    }

    if ( this->visible ) {
        glfwShowWindow ( this->window );
    } else {
        glfwHideWindow ( this->window );
    }

    if ( this->fullscreenEnabled ) {
        glfwGetFramebufferSize ( this->window, &this->bufferSize.X, &this->bufferSize.Y );
        glViewport ( 0, 0, this->bufferSize.X, this->bufferSize.Y );
    } 
    
    else {
        glfwSetWindowSize(this->window, this->dimensions.width, this->dimensions.height);
        glfwSetWindowPos(this->window, this->dimensions.xPos, this->dimensions.yPos);
    }

    // framerate
    glfwSwapInterval( this->vSyncEnabled ? 1 : 0 );
    this->frameTime = highResClock::duration ( 
        static_cast<highResClock::rep>(highResClock::period::den / this->maxFrameRate) 
    );

    this->changedFlags = 0;
    return true;

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
            return false;
        }

        this->oldDimensions = this->dimensions;
        this->window = createGlfwWindow(this->winTitle, this->fullscreenEnabled, NULL, this->dimensions);
        
        if (!this->window) {
            std::cout << "Failed to open GLFW window" << std::endl;
            return false;
        } 
        
        else {
            windowMap[this->window] = this; 
            windowCount++;
        }

        if (!this->initWindow()) {
            std::cout << "Failed to initialize window" << std::endl;
            return false;
        }
        
        this->thread = new std::thread(AppWindow::run, this);
        mainThreadRunner->addChild(this->thread);
        
        return true;

    });
}

void AppWindow::destroy ( bool force ) {

    if ( this->isDestroyed ) {
        return;
    }

    std::lock_guard<std::mutex> lockA(this->localMtx);
    std::lock_guard<std::mutex> lockB(global_win_mtx);

    if ( (!force) && ((!this->thread) || (this->thread->get_id() != std::this_thread::get_id()))) {
        this->shouldDestroy = true;
        return;
    }

    mainThreadRunner->scheduleAndWait<void> ( [this]() -> void {

        if ( this->window ) {
            glfwDestroyWindow(this->window); 
            windowMap.erase(this->window);

            this->window = nullptr;
            windowCount--;
        }

        this->isDestroyed = true;

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

// Only call this well this->fullscreenEnabled has changed.
void AppWindow::iSetFullScreen ( ) {

    mainThreadRunner->scheduleAndWait<void>([this]() -> void {

        GlfwContextLock lock ( this->window );
        toggle_callbacks ( this->window, false ); // callbacks would deadlock otherwise.

        if ( this->fullscreenEnabled ) {

            GLFWmonitor* monitor = this->getMonitor();
            MonitorData* data = this->getMonitorData();

            if ( (!data) || (!monitor) ) { 
                std::cout << "Failed to enable fullscreen" << std::endl;
                toggle_callbacks ( this->window, true );
                return;
            }

            this->oldDimensions = this->dimensions;
            this->dimensions = data->getDimensions();

            glfwSetWindowMonitor ( this->window, monitor, 
                data->xPos, data->yPos, data->width, data->height, data->refreshRate );

        } else {

            this->dimensions = this->oldDimensions;

            glfwSetWindowMonitor ( this->window, NULL, this->dimensions.xPos, this->dimensions.yPos, 
                this->dimensions.width, this->dimensions.height, GLFW_DONT_CARE);

        }

        glfwGetFramebufferSize ( this->window, &this->bufferSize.X, &this->bufferSize.Y );
        glViewport ( 0, 0, this->bufferSize.X, this->bufferSize.Y );

        toggle_callbacks ( this->window, true );

    });

}

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

    glfwMakeContextCurrent(this->window);    

    highResClock::time_point frameStart, frameEnd;
    highResClock::duration timeElapsed;
    float deltaTime = 0;

    while(!this->shouldDestroy && !glfwWindowShouldClose(this->window)) {

        if ( this->changedFlags ) {
            this->applyChanges();
        }

        if ( !this->visible ) { 
            continue; // no point in redering something that can not be seen.
        }
        
        frameStart = highResClock::now();

        this->render(deltaTime);
        glfwSwapBuffers(this->window);

        frameEnd = highResClock::now();
        timeElapsed = frameEnd - frameStart;

        if ( frameTime > timeElapsed ) {
            sleepFor ( frameTime - timeElapsed );

            frameEnd = highResClock::now();
            timeElapsed = frameEnd - frameStart;
        } 
                    
        deltaTime = std::chrono::duration<float>(timeElapsed).count();

    }

    this->isActive = false;
    this->destroy();

}

void AppWindow::flipFlag ( uint8_t flag ) {
    this->changedFlags ^= flag;
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

        this->flipFlag(VISIBILITY_CHANGED_FLAG);
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

        this->flipFlag(VSYNC_CHANGED_FLAG);
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
        
        this->flipFlag(FULLSCREEN_CHANGED_FLAG);
        this->fullscreenEnabled = enabled;
    }
}

void AppWindow::setBufferSize ( int width, int height ) {

    if ( this->bufferSize.X != width && this->bufferSize.Y != height ) {
        std::lock_guard<std::mutex> lockA (this->localMtx);
        GlfwContextLock lockB ( this->window );

        glViewport(0, 0, width, height);

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
