
#include "QuitKey.h"

void QuitKey::onPressed ( AppWindow* win, int mods ) {
    this->shouldQuit = true;
    this->window = win;
}

void QuitKey::onRepeated ( AppWindow* win, int mods ) {
    this->shouldQuit = false;
}

void QuitKey::onReleased ( AppWindow* win, int mods ) {

    if ( this->shouldQuit && this->window ) {
        this->window->destroy();
    }

}