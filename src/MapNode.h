#pragma once

#include <iostream>
#include "raylib.h"

class MapNode
{
protected:
    Vector2 position;

public:
    MapNode(int x, int y);
    Vector2 Position();
    int Cost();
    bool BlocksMovement();
    bool BlocksVision();
};