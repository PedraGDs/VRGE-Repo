
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
        std::atomic<uint8_t> changedFlags = 0;
        std::thread* thread = nullptr;
        GLFWwindow* window = nullptr;
        std::mutex localMtx{};
        
        Rect2d dimensions = defaultWindowDimensions;
        float maxFrameRate = INFINITY;
        bool fullscreenEnabled = false;
        bool shouldDestroy = false;
        bool vSyncEnabled = true;
        char* winTitle;

        void run();
        void render( float deltaTime );

        void iSetFlag ( uint8_t flag, bool enabled );
        bool isFlagEnabled ( uint8_t flag );

        void iSetFullScreen ( );

    public:
        inline Window ( char* title, Rect2d dims ) noexcept: dimensions(dims), winTitle(title) { }
        inline Window ( char* title ) noexcept: winTitle(title) { }

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

        void setTitle ( char* newTitle );
        char* getTitle ( );

        void setFullScreen ( bool enabled );
        bool isFullScreen ();
        
};