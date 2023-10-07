#include "MapNode.h"

MapNode::MapNode(int x, int y)
{
    position.x = static_cast<float>(x);
    position.y = static_cast<float>(y);
}

Vector2 MapNode::Position()
{
    return position;
}

int MapNode::Cost()
{
    return 0;
}

bool MapNode::BlocksMovement()
{
    return false;
}

bool MapNode::BlocksVision()
{
    return false;
}
