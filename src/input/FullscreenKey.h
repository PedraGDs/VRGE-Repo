
#pragma once

#include "InputHandler.h"

class FullScreenKey: public KeyBinding {

    private:
        AppWindow* window = nullptr;
        bool toggleFullScreen = false;

    public:
        FullScreenKey () { };

        void onRepeated ( AppWindow* window, int mods ) override;
        void onReleased ( AppWindow* window, int mods ) override;
        void onPressed ( AppWindow* window, int mods ) override;

};
