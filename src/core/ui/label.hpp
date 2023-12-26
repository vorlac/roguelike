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

    class label : public ui::widget
    {
    public:
        label(ui::widget* parent, const std::string& caption,
              const std::string& font = font::name::mono, i32 font_size = -1);

        const std::string& caption() const
        {
            return m_caption;
        }

        const std::string& font() const
        {
            return m_font;
        }

        ds::color<f32> color() const
        {
            return m_color;
        }

        void set_caption(const std::string& caption)
        {
            m_caption = caption;
        }

        void set_font(const std::string& font)
        {
            m_font = font;
        }

        void set_color(ds::color<f32> color)
        {
            m_color = color;
        }

        void set_callback(auto&& callable)
        {
            m_callback = std::move(callable);
        }

        virtual ds::dims<i32> preferred_size(NVGcontext* nvg_context) const override;
        virtual void set_theme(ui::theme* theme) override;
        virtual void draw(NVGcontext* nvg_context) override;

    protected:
        std::string m_caption{};
        std::string m_font{};
        ds::color<f32> m_color{ rl::Colors::Yellow };
        std::function<void()> m_callback;
    };
}
