#pragma once

#include <functional>
#include <string>

#include "ds/color.hpp"
#include "ds/dims.hpp"
#include "ui/theme.hpp"
#include "ui/widget.hpp"

namespace rl::ui {

    class Label final : public Widget
    {
    public:
        explicit Label(std::string text, f32 font_size = text::font::InvalidSize,
                       Align alignment = Align::HLeft | Align::VMiddle);
        explicit Label(Widget* parent, std::string text, f32 font_size = text::font::InvalidSize,
                       Align alignment = Align::HLeft | Align::VMiddle);

        const std::string& font() const;
        const std::string& text() const;
        const ds::color<f32>& color() const;
        Align text_alignment() const;

        void set_text(const std::string& text);
        void set_font(const std::string& font);
        void set_text_alignment(Align alignment);
        void set_color(const ds::color<f32>& color);
        void set_callback(const std::function<void()>& callable);

    public:
        virtual ds::dims<f32> preferred_size() const override;
        virtual void set_theme(ui::Theme* theme) override;
        virtual void draw() override;

    protected:
        std::string m_text{};
        std::string m_text_font{};
        bool m_font_autosizing{ false };
        Align m_text_alignment{ Align::HLeft | Align::VMiddle };
        ds::color<f32> m_text_color{ Colors::White };
        std::function<void()> m_callback;
    };
}
