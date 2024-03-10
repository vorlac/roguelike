#include "core/keyboard.hpp"
#include "core/mouse.hpp"
#include "core/ui/theme.hpp"
#include "core/ui/widgets/slider.hpp"
#include "utils/math.hpp"

namespace rl::ui {

    Slider::Slider(Widget* parent)
        : Widget(parent)
    {
    }

    f32 Slider::value() const
    {
        return m_value;
    }

    void Slider::set_value(const f32 value)
    {
        m_value = value;
    }

    const ds::color<f32>& Slider::highlight_color() const
    {
        return m_highlight_color;
    }

    void Slider::set_highlight_color(const ds::color<f32>& highlight_color)
    {
        m_highlight_color = highlight_color;
    }

    std::pair<f32, f32> Slider::range() const
    {
        return m_range;
    }

    void Slider::set_range(const std::pair<f32, f32> range)
    {
        m_range = range;
    }

    std::pair<f32, f32> Slider::highlighted_range() const
    {
        return m_highlighted_range;
    }

    void Slider::set_highlighted_range(const std::pair<f32, f32> highlighted_range)
    {
        m_highlighted_range = highlighted_range;
    }

    const std::function<void(f32)>& Slider::callback() const
    {
        return m_callback;
    }

    void Slider::set_callback(const std::function<void(f32)>& callback)
    {
        m_callback = callback;
    }

    const std::function<void(f32)>& Slider::final_callback() const
    {
        return m_final_callback;
    }

    void Slider::set_final_callback(const std::function<void(f32)>& callback)
    {
        m_final_callback = callback;
    }

    ds::dims<f32> Slider::preferred_size() const
    {
        return ds::dims{ 70.0f, 16.0f };
    }

    bool Slider::on_mouse_drag(const Mouse& mouse, const Keyboard& kb)
    {
        if (!m_enabled)
            return false;

        auto&& mouse_pos{ mouse.pos() };
        const f32 kr{ m_rect.size.height * 0.4f };
        constexpr f32 kshadow{ 3.0f };
        const f32 start_x{ kr + kshadow + m_rect.pt.x - 1.0f };
        const f32 width_x{ m_rect.size.width - 2.0f * (kr + kshadow) };
        const f32 old_value{ m_value };

        f32 value{ (mouse_pos.x - start_x) / width_x };

        value = value * (m_range.second - m_range.first) + m_range.first;
        m_value = std::min(std::max(value, m_range.first), m_range.second);

        if (m_callback != nullptr && m_value != old_value)
            m_callback(m_value);

        return true;
    }

    bool Slider::on_mouse_button_pressed(const Mouse& mouse, const Keyboard& kb)
    {
        if (!m_enabled)
            return false;

        auto&& mouse_pos{ mouse.pos() };
        const f32 kr{ m_rect.size.height * 0.4f };
        constexpr f32 kshadow{ 3.0f };
        const f32 start_x{ kr + kshadow + m_rect.pt.x - 1.0f };
        const f32 width_x{ m_rect.size.width - 2.0f * (kr + kshadow) };
        const f32 old_value{ m_value };

        f32 value{ (mouse_pos.x - start_x) / width_x };

        value = value * (m_range.second - m_range.first) + m_range.first;
        m_value = std::min(std::max(value, m_range.first), m_range.second);

        if (m_callback != nullptr && m_value != old_value)
            m_callback(m_value);

        return true;
    }

    bool Slider::on_mouse_button_released(const Mouse& mouse, const Keyboard& kb)
    {
        if (!m_enabled)
            return false;

        const auto mouse_pos{ mouse.pos() };
        const f32 kr{ m_rect.size.height * 0.4f };
        constexpr f32 kshadow{ 3.0f };
        const f32 start_x{ kr + kshadow + m_rect.pt.x - 1.0f };
        const f32 width_x{ m_rect.size.width - 2.0f * (kr + kshadow) };
        const f32 old_value{ m_value };

        f32 value{ (mouse_pos.x - start_x) / width_x };

        value = value * (m_range.second - m_range.first) + m_range.first;
        m_value = std::min(std::max(value, m_range.first), m_range.second);

        if (m_callback != nullptr && m_value != old_value)
            m_callback(m_value);

        if (m_final_callback != nullptr)
            m_final_callback(m_value);

        return true;
    }

    void Slider::draw()
    {
        const auto context{ m_renderer->context() };
        const ds::point center{ ds::rect{ m_rect.pt, m_rect.size }.centroid() };
        const f32 kr{ m_rect.size.height * 0.4f };
        constexpr f32 kshadow{ 3.0f };

        const f32 start_x{ kr + kshadow + m_rect.pt.x };
        const f32 width_x{ m_rect.size.width - 2.0f * (kr + kshadow) };

        const ds::vector2 knob_pos{
            start_x + (m_value - m_range.first) / (m_range.second - m_range.first) * width_x,
            center.y + 0.5f,
        };

        const nvg::PaintStyle bg{ nvg::box_gradient(
            context, start_x, center.y - 3.0f + 1.0f, width_x, 6.0f, 3.0f, 3.0f,
            ds::color<f32>{ 0, 0, 0, m_enabled ? 32 : 10 },
            ds::color<f32>{ 0, 0, 0, m_enabled ? 128 : 210 }) };

        nvg::begin_path(context);
        nvg::rounded_rect(context, start_x, center.y - 3.0f + 1.0f, width_x, 6.0f, 2.0f);
        nvg::fill_paint(context, bg);
        nvg::fill(context);

        if (m_highlighted_range.second != m_highlighted_range.first)
        {
            nvg::begin_path(context);
            nvg::rounded_rect(context, start_x + m_highlighted_range.first * m_rect.size.width,
                              center.y - kshadow + 1.0f,
                              width_x * (m_highlighted_range.second - m_highlighted_range.first),
                              kshadow * 2.0f, 2.0f);
            nvg::fill_color(context, m_highlight_color);
            nvg::fill(context);
        }

        const nvg::PaintStyle knob_shadow{
            nvg::radial_gradient(context, knob_pos.x, knob_pos.y, kr - kshadow, kr + kshadow,
                                 ds::color<f32>{ 0, 0, 0, 64 }, m_theme->transparent),
        };

        nvg::begin_path(context);
        nvg::rect(context, knob_pos.x - kr - 5.0f, knob_pos.y - kr - 5.0f, kr * 2.0f + 10.0f,
                  kr * 2.0f + 10.0f + kshadow);
        nvg::circle(context, knob_pos.x, knob_pos.y, kr);
        nvg::path_winding(context, nvg::Solidity::Hole);
        nvg::fill_paint(context, knob_shadow);
        nvg::fill(context);

        const nvg::PaintStyle knob{ nvg::linear_gradient(
            context, m_rect.pt.x, center.y - kr, m_rect.pt.x, center.y + kr, m_theme->border_light,
            m_theme->border_medium) };
        const nvg::PaintStyle knob_reverse{ nvg::linear_gradient(
            context, m_rect.pt.x, center.y - kr, m_rect.pt.x, center.y + kr, m_theme->border_medium,
            m_theme->border_light) };

        nvg::begin_path(context);
        nvg::circle(context, knob_pos.x, knob_pos.y, kr);
        nvg::stroke_color(context, m_theme->border_dark);
        nvg::fill_paint(context, knob);
        nvg::stroke(context);
        nvg::fill(context);
        nvg::begin_path(context);
        nvg::circle(context, knob_pos.x, knob_pos.y, kr / 2.0f);
        nvg::fill_color(context, ds::color<f32>{ 150, 150, 150, m_enabled ? 255 : 100 });
        nvg::stroke_paint(context, knob_reverse);
        nvg::stroke(context);
        nvg::fill(context);
    }
}
