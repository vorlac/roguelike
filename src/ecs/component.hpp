#pragma once

#include <string>

namespace rl::component
{
    inline namespace transform
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

        inline namespace shape
        {
            struct Rectangle
            {
                Position tl{ 0.0, 0.0 };
            };
        }

        inline namespace character
        {
            struct Character
            {
                bool alive{ false };
            };

            struct Health
            {
                int amount{ 0 };
            };
        }

        inline namespace ui
        {
            struct Button
            {
                std::string text{};
            };
        }
    }
}
