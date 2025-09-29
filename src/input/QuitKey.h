
#pragma once

#include "InputHandler.h"

class QuitKey: public KeyBinding {

    private:
        AppWindow* window = nullptr;
        bool shouldQuit = false;

    public:
        QuitKey () { };

        void onRepeated ( AppWindow* window, int mods ) override;
        void onReleased ( AppWindow* window, int mods ) override;
        void onPressed ( AppWindow* window, int mods ) override;

};
