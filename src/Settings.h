#pragma once

#include "Keyboard.h"

class Settings
{
public:
    KeyboardConfiguration keyboard;

    Settings()
    {
        keyboard = LoadKeyboardConfigfuration();
    }
};
