#pragma once

#include "raylib.h"

class GraphNode
{
    virtual Vector2 Position() =0;
    virtual int Cost() = 0;
    virtual bool BlocksMovement() = 0;
    virtual bool BlocksVision() = 0;
};