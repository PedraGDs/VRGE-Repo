
#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "core/AppWindow.h"

void key_callback ( GLFWwindow* window, int key, int scancode, int action, int mods );

class KeyBinding {

    private:
        bool isRegistered = false;
        int keyId = 0;

    public:
        KeyBinding() { };

        void registerKeybind ( );

        void setKeyId ( int keyId );
        int getKeyId ( );

        virtual void onRepeated ( AppWindow* window, int mods );
        virtual void onReleased ( AppWindow* window, int mods );
        virtual void onPressed ( AppWindow* window, int mods );

};

