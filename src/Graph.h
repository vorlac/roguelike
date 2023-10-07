#pragma once

#include <vector>

#include "raylib.h"
#include "GraphNode.h"


class Graph2D
{
    public:
        virtual GraphNode GetNode(Vector2 t) = 0;
        virtual std::vector<GraphNode> GetNeighborNodes(GraphNode n) = 0;
        virtual int GetDistance(GraphNode a, GraphNode b) = 0;
        virtual bool CheckBounds(Vector2 n) = 0;
        // virtual bool CheckBounds(Vector3 node);
        virtual bool IsMovementBlocked(Vector2 n) = 0;
        virtual bool IsMovementBlocked(GraphNode n) = 0;
};


