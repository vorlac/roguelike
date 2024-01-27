#pragma once

#include <functional>
#include <string>

#include "core/ui/widget.hpp"
#include "ds/color.hpp"
#include "ds/dims.hpp"
#include "ds/refcounted.hpp"
#include "ds/shared.hpp"
#include "ds/vector2d.hpp"

namespace rl::ui {

    class Label : public Widget
    {
    public:
        Label(Widget* parent, std::string caption, std::string font = font::name::mono,
              f32 font_size = -1.0f);

        std::string font() const;
        std::string text() const;
        ds::color<f32> color() const;

        void set_text(std::string text);
        void set_font(std::string font);
        void set_color(ds::color<f32> color);
        void set_callback(std::function<void()> callable);

    public:
        virtual ds::dims<f32> preferred_size() const override;
        virtual void set_theme(ui::Theme* theme) override;
        virtual void draw() override;

    protected:
        std::string m_font{};
        std::string m_text{};
        ds::color<f32> m_color{ rl::Colors::Yellow };
        std::function<void()> m_callback;
    };
}
