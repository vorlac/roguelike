#pragma once

#include <functional>
#include <string>

#include "core/ui/theme.hpp"
#include "core/ui/widget.hpp"
#include "ds/color.hpp"
#include "ds/dims.hpp"

namespace rl::ui {

    class Label final : public Widget
    {
    public:
        explicit Label(std::string text, f32 font_size = -1.0f);
        explicit Label(Widget* parent, std::string text, f32 font_size = -1.0f);

        const std::string& font() const;
        const std::string& text() const;
        const ds::color<f32>& color() const;

        void set_text(const std::string& text);
        void set_font(const std::string& font);
        void set_color(const ds::color<f32>& color);
        void set_callback(const std::function<void()>& callable);

    public:
        virtual ds::dims<f32> preferred_size() const override;
        virtual void set_theme(ui::Theme* theme) override;
        virtual void draw() override;

    protected:
        std::string m_text{};
        std::string m_text_font{};
        ds::color<f32> m_text_color{ rl::Colors::White };
        std::function<void()> m_callback;
    };
}
