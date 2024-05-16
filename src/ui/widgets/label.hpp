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
        explicit Label(
            std::string text,
            f32 font_size = text::font::InvalidSize,
            Align alignment = Align::HLeft | Align::VMiddle);
        explicit Label(
            Widget* parent, std::string text,
            f32 font_size = text::font::InvalidSize,
            Align alignment = Align::HLeft | Align::VMiddle);

        [[nodiscard]] std::string_view font() const;
        [[nodiscard]] std::string_view text() const;
        [[nodiscard]] ds::color<f32> color() const;
        [[nodiscard]] Align text_alignment() const;

        void set_text(std::string text);
        void set_font(std::string_view font);
        void set_text_alignment(Align alignment);
        void set_color(ds::color<f32> color);
        void set_callback(const std::function<void()>& callable);

    public:
        virtual ds::dims<f32> preferred_size() const override;
        virtual void set_theme(Theme* theme) override;
        virtual void draw() override;

    protected:
        std::string m_text{};
        std::string m_font{ m_theme->label_font_name };
        bool m_font_autosizing{ false };
        Align m_text_alignment{ Align::HLeft | Align::VMiddle };
        ds::color<f32> m_text_color{ m_theme->label_font_color };
        ds::color<f32> m_text_outline_color{ m_theme->text_shadow };
        std::function<void()> m_callback{ nullptr };
    };
}
