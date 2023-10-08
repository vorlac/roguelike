#pragma once

#include "math/dimension2d.hpp"
#include "math/vector2d.hpp"

#include <raylib.h>
#include <string>
#include <utility>

namespace rl
{
    class Window
    {
    public:
        Window(dims2i dimensions = { .width = 1024, .height = 768 }, std::string title = "")
            : m_dims{ dimensions }
            , m_title{ title }
        {
            InitWindow(m_dims.width, m_dims.height, m_title.c_str());
        }

        ~Window()
        {
            CloseWindow();
        }

        bool should_close() const
        {
            // Detect window close button or ESC key
            return WindowShouldClose();
        }

        dims2i screen_dims() const
        {
            return m_dims;
        }

    private:
        dims2i m_dims{
            .width = 1024,
            .height = 768,
        };

        std::string m_title{ "" };
    };

}
