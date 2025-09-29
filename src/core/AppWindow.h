
#pragma once

#include <mutex>
#include <cmath>
#include <thread>
#include <atomic>
#include <chrono>
#include <glad/glad.h>
#include <glfw/glfw3.h>
#include "util/Colors.h"
#include "util/Vectors.h"
#include "util/math/Rect2d.h"

static constexpr Rect2d defaultAppWindowDimensions( 0, 0, 854, 480 );
static constexpr Color4f AppWindowBackgroundColor(0.07F, 0.13F, 0.17F, 1.0F);

// GLFW && Glad
// Must only be called from the main thread.
bool initGlfw ();
void stopGlfw ();

class AppWindow; // foward
AppWindow* getAppWindow ( GLFWwindow* window );

class AppWindow {
    
    private:
        std::atomic<uint8_t> changedFlags = 0;
        std::thread* thread = nullptr;
        GLFWwindow* window = nullptr;
        std::mutex localMtx{};
        Vector2i bufferSize{};

        std::chrono::high_resolution_clock::duration frameTime{};
        Rect2d dimensions = defaultAppWindowDimensions;
        float maxFrameRate = INFINITY;
        bool fullscreenEnabled = false;
        bool shouldDestroy = false;
        bool vSyncEnabled = true;
        const char* winTitle;

        bool isDestroyed = false;
        bool isActive = false;
        bool visible = true;

        void run();
        void render( float deltaTime );

        void iSetFullScreen ( );

        void setFlag ( uint8_t flag, bool enabled );
        bool isFlagEnabled ( uint8_t flag );
        void applyChanges ( );


    public:
        bool initializeCentered = true;

        inline AppWindow ( const char* title, Rect2d dims ) noexcept: dimensions(dims), winTitle(title) { }
        inline AppWindow ( const char* title ) noexcept: winTitle(title) { }

        ~AppWindow ();

        bool init ( );
        void destroy ();

        std::thread* getThread ();
        GLFWmonitor* getMonitor ();

        void setDimensions ( Rect2d dimensions );
        Rect2d getDimensions ();

        Vector2i getSize ();
        Vector2i getPos ();

        void setSize ( int width, int height, bool isCallback );
        inline void setSize ( int width, int height ) { this->setSize(width, height, false); }
        inline void setSize ( Vector2i size ) { this->setSize(size.X, size.Y); }

        void setPos ( int xPos, int yPos, bool isCallback );
        inline void setPos ( int xPos, int yPos ) { this->setPos(xPos, yPos); }
        inline void setPos ( Vector2i pos ) { this->setPos(pos.X, pos.Y); }

        void setVisible ( bool visible );
        bool isVisible ( );

        void setMaxFrameRate ( float frameRate );
        float getMaxFrameRate ();

        void setVSyncEnabled ( bool enabled );
        bool isVSyncEnabled ();

        void setTitle ( const char* newTitle );
        const char* getTitle ( );

        void setFullScreen ( bool enabled );
        bool isFullScreen ();

        void setBufferSize ( int width, int height ); // callback
        Vector2i getBufferSize ();

        
};
