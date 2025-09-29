
#include "QuitKey.h"


void registerKeyBinds ( ) {

    KeyBinding* key = new QuitKey();
    key->setKeyId(GLFW_KEY_ESCAPE);
    key->registerKeybind();

}



