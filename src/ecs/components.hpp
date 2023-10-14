#pragma once

#include <string>

namespace rl::components
{
    struct Position
    {
        float x{ 0.0 };
        float y{ 0.0 };
    };

    struct Velocity
    {
        float x{ 0.0 };
        float y{ 0.0 };
    };

    struct AngleDegrees
    {
        float angle{ 0.0 };
    };

    struct AngleRadians
    {
        float angle{ 0.0 };
    };

    struct Scale
    {
        float x{ 0.0 };
        float y{ 0.0 };
    };

    struct Dimensions
    {
        float width{ 0.0 };
        float height{ 0.0 };
    };

    struct Rectangle
    {
        Position tl{ 0.0, 0.0 };
    };

    struct Character
    {
        bool alive{ false };
    };

    struct Health
    {
        int amount{ 0 };
    };

    struct Button
    {
        std::string text{};
    };
}
