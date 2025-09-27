
#pragma once

#include <cmath>
#include <mutex>
#include <thread>
#include <atomic>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "util/Colors.h"
#include "util/Vectors.h"
#include "util/math/Rect2d.h"

static constexpr Rect2d defaultWindowDimensions( 0, 0, 1024, 720 );
static constexpr Color4f windowBackgroundColor(0.07F, 0.13F, 0.17F, 1.0F);

class Window {
    
    private:
        Rect2d dimensions = defaultWindowDimensions;
        std::thread* thread = nullptr;
        GLFWwindow* window = nullptr;

        std::atomic<float> maxFrameRate = INFINITY;
        bool frameRateChanged = false;
        bool shouldDestroy = false; // atomic rw
        bool vSyncEnabled = true;
        bool vSynChanged = false;

        void run();
        void render( float deltaTime );

    public:
        constexpr inline Window ( Rect2d dims ) noexcept: dimensions(dims) { }
        constexpr inline Window ( ) noexcept { }

        bool init ();
        void destroy ();

        std::thread* getThread ();

        void setDimensions ( Rect2d dimensions );
        Rect2d getDimensions ();

        void setMaxFrameRate ( float frameRate );
        void setVSyncEnabled ( bool enabled );

        float getMaxFrameRate ();
        bool isVSyncEnabled ();

};