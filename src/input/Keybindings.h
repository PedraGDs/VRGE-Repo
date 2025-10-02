
#include "QuitKey.h"
#include "FullScreenKey.h"

void registerKeyBinds ( ) {

    KeyBinding* quitKey = new QuitKey();
    quitKey->setKeyId(GLFW_KEY_ESCAPE);
    quitKey->registerKeybind();

    KeyBinding* fsKey = new FullScreenKey();
    fsKey->setKeyId(GLFW_KEY_F11);
    fsKey->registerKeybind();

}



