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

    class Label : public ui::Widget
    {
    public:
        Label(ui::Widget* parent, const std::string& caption,
              const std::string& font = font::name::mono, i32 font_size = -1);

        std::string font() const;
        std::string caption() const;
        ds::color<f32> color() const;

        void set_caption(const std::string& caption);
        void set_font(const std::string& font);
        void set_color(ds::color<u8> color);
        void set_callback(std::function<void()> callable);

    public:
        virtual ds::dims<i32> preferred_size(NVGcontext* nvg_context) const override;
        virtual void set_theme(ui::Theme* theme) override;
        virtual void draw(NVGcontext* nvg_context) override;

    protected:
        std::string m_font{};
        std::string m_caption{};
        ds::color<f32> m_color{ rl::Colors::Yellow };
        std::function<void()> m_callback;
    };
}
