
#include "FullscreenKey.h"

void FullScreenKey::onPressed ( AppWindow* win, int mods ) {
    this->toggleFullScreen = true;
    this->window = win;
}

void FullScreenKey::onRepeated ( AppWindow* win, int mods ) {
    this->toggleFullScreen = false;
}

void FullScreenKey::onReleased ( AppWindow* win, int mods ) {
    if ( this->toggleFullScreen && this->window ) {
        this->window->setFullScreen(!this->window->isFullScreen());
    }
}