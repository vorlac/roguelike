#include "GameMap.h"

GameMap::GameMap(int w, int h)
{
    size.x = static_cast<float>(w);
    size.y = static_cast<float>(h);

    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++)
            nodes.push_back(MapNode(x, y));
}

MapNode GameMap::GetNode(Vector2 t)
{
    int vPos = static_cast<int>((size.y * t.y) + t.x);
    return nodes[vPos];
}

std::vector<MapNode> GameMap::GetNeighborNodes(MapNode n)
{
    return std::vector<MapNode>();
}

int GameMap::GetDistance(MapNode a, MapNode b)
{
    // implement uclidian distance
    return 0;
}

bool GameMap::CheckInBounds(Vector2 n)
{
    if (n.x < size.x && n.x >= 0 && n.y < size.y && n.y >= 0)
        return true;
    else
        return false;
}

bool GameMap::IsMovementBlocked(Vector2 n)
{
    return false;
}

bool GameMap::IsMovementBlocked(MapNode n)
{
    return false;
}
