
#include "InputHandler.h"
#include <unordered_map>

std::unordered_map<int, KeyBinding*> keyMap;

void KeyBinding::setKeyId ( int keyId ) {
    
    if ( this->isRegistered ) {
        keyMap[this->keyId] = nullptr;
    }

    keyMap[keyId] = this;
    this->keyId = keyId;

}

int KeyBinding::getKeyId ( ) {
    return this->keyId;
}

void KeyBinding::registerKeybind ( ) {
    keyMap[this->keyId] = this;
    this->isRegistered = true;
}

void key_callback ( GLFWwindow* window, int keyId, int scancode, int action, int mods ) {

    KeyBinding* key = keyMap[keyId];
    if ( !key ) { return; }

    AppWindow* appWindow = getAppWindow(window);

    switch (action) {
        case GLFW_PRESS:
            key->onPressed (appWindow, mods);
            break;
        
        case GLFW_RELEASE:
            key->onReleased (appWindow, mods);
            break;
        
        case GLFW_REPEAT:
            key->onRepeated (appWindow, mods);
            break;

        default:
            break;
    }

}
