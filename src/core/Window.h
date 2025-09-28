
#pragma once

#include <cmath>
#include <thread>
#include <atomic>
#include <GLFW/glfw3.h>
#include "util/Colors.h"
#include "util/math/Rect2d.h"

static constexpr Rect2d defaultWindowDimensions( 0, 0, 1024, 720 );
static constexpr Color4f windowBackgroundColor(0.07F, 0.13F, 0.17F, 1.0F);

class Window {
    
    private:
        Rect2d dimensions = defaultWindowDimensions;
        std::thread* thread = nullptr;
        GLFWwindow* window = nullptr;
        char* winTitle;

        std::atomic<float> maxFrameRate = INFINITY;
        bool fullscreenEnabled = false;
        bool fullscreenChanged = false;
        bool frameRateChanged = false;
        bool shouldDestroy = false;
        bool titleChanged = false;
        bool vSyncEnabled = true;
        bool vSynChanged = false;

        void run();
        void render( float deltaTime );

    public:
        constexpr inline Window ( char* title, Rect2d dims ) noexcept: dimensions(dims), winTitle(title){}
        constexpr inline Window ( char* title ) noexcept :winTitle(title) { }

        bool init ();
        void destroy ();

        std::thread* getThread ();

        void setDimensions ( Rect2d dimensions );
        Rect2d getDimensions ();

        void setMaxFrameRate ( float frameRate );
        float getMaxFrameRate ();

        void setVSyncEnabled ( bool enabled );
        bool isVSyncEnabled ();

        void setTitle ( char* newTitle );
        char* getTitle ( );

        void setFullScreen ( bool enabled );
        bool isFullScreen ();

        GLFWmonitor* getMonitor ();
        
};