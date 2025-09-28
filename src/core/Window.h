
#pragma once

#include <cmath>
#include <thread>
#include <atomic>
#include <mutex>
#include <glad/glad.h>
#include <glfw/glfw3.h>
#include "util/Colors.h"
#include "util/math/Rect2d.h"

static constexpr Rect2d defaultWindowDimensions( 0, 0, 1024, 720 );
static constexpr Color4f windowBackgroundColor(0.07F, 0.13F, 0.17F, 1.0F);

// GLFW && Glad
// Must only be called from the main thread.
bool initGlfw ();
void stopGlfw ();

class Window {
    
    private:
        std::atomic<uint8_t> changedFlags = 0;
        std::thread* thread = nullptr;
        GLFWwindow* window = nullptr;
        std::mutex localMtx{};
        
        Rect2d dimensions = defaultWindowDimensions;
        float maxFrameRate = INFINITY;
        bool fullscreenEnabled = false;
        bool shouldDestroy = false;
        bool vSyncEnabled = true;
        const char* winTitle;

        bool isDestroyed = false;
        bool isActive = false;

        void run();
        void render( float deltaTime );

        void iSetFlag ( uint8_t flag, bool enabled );
        bool isFlagEnabled ( uint8_t flag );

        void iSetFullScreen ( );

    public:
        inline Window ( const char* title, Rect2d dims ) noexcept: dimensions(dims), winTitle(title) { }
        inline Window ( const char* title ) noexcept: winTitle(title) { }

        ~Window ();

        bool init ();
        void destroy ();

        std::thread* getThread ();
        GLFWmonitor* getMonitor ();

        void setDimensions ( Rect2d dimensions );
        Rect2d getDimensions ();

        void setMaxFrameRate ( float frameRate );
        float getMaxFrameRate ();

        void setVSyncEnabled ( bool enabled );
        bool isVSyncEnabled ();

        void setTitle ( const char* newTitle );
        const char* getTitle ( );

        void setFullScreen ( bool enabled );
        bool isFullScreen ();
        
};