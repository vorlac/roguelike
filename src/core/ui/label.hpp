#pragma once

#include <functional>
#include <string>

#include "core/ui/widget.hpp"
#include "ds/color.hpp"
#include "ds/dims.hpp"

namespace rl::ui {
    class Label : public Widget
    {
    public:
        Label(Widget* parent, std::string text, const std::string_view& font = Font::Name::Mono,
              f32 font_size = -1.0f);

        const std::string_view& font() const;
        std::string text() const;
        ds::color<f32> color() const;

        void set_text(const std::string& text);
        void set_font(const std::string& font);
        void set_color(const ds::color<f32>& color);
        void set_callback(const std::function<void()>& callable);

    public:
        ds::dims<f32> preferred_size() const override;
        void set_theme(ui::Theme* theme) override;
        void draw() override;

    protected:
        std::string m_text{};
        std::string_view m_font{};
        ds::color<f32> m_color{ rl::Colors::Yellow };
        std::function<void()> m_callback;
    };
}
