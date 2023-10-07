#pragma once
#include <vector>

#include "MapNode.h"


class GameMap
{
    std::vector<MapNode> nodes;
    Vector2 size;
    public:
        GameMap(int w, int h);
        MapNode GetNode(Vector2 t);
        std::vector<MapNode> GetNeighborNodes(MapNode n);
        int GetDistance(MapNode a, MapNode b);
        bool CheckInBounds(Vector2 n);
        bool IsMovementBlocked(Vector2 n);
        bool IsMovementBlocked(MapNode n);
};