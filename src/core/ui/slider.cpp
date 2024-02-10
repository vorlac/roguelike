#include "core/keyboard.hpp"
#include "core/mouse.hpp"
#include "core/ui/slider.hpp"
#include "core/ui/theme.hpp"

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
        const f32 kr{ m_size.height * 0.4f };
        const f32 kshadow{ 3.0f };
        const f32 start_x{ kr + kshadow + m_pos.x - 1.0f };
        const f32 width_x{ m_size.width - 2.0f * (kr + kshadow) };
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
        const f32 kr{ m_size.height * 0.4f };
        const f32 kshadow{ 3.0f };
        const f32 start_x{ kr + kshadow + m_pos.x - 1.0f };
        const f32 width_x{ m_size.width - 2.0f * (kr + kshadow) };
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

        auto&& mouse_pos{ mouse.pos() };
        const f32 kr{ m_size.height * 0.4f };
        const f32 kshadow{ 3.0f };
        const f32 start_x{ kr + kshadow + m_pos.x - 1.0f };
        const f32 width_x{ m_size.width - 2.0f * (kr + kshadow) };
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
        auto&& context{ m_renderer->context() };
        ds::point<f32> center{ ds::rect{ m_pos, m_size }.centroid() };
        f32 kr{ m_size.height * 0.4f };
        f32 kshadow{ 3.0f };

        f32 start_x{ kr + kshadow + m_pos.x };
        f32 width_x{ m_size.width - 2.0f * (kr + kshadow) };

        ds::vector2<f32> knob_pos{
            start_x + (m_value - m_range.first) / (m_range.second - m_range.first) * width_x,
            center.y + 0.5f,
        };

        nvg::NVGpaint bg{ nvg::box_gradient(
            context, start_x, center.y - 3.0f + 1.0f, width_x, 6.0f, 3.0f, 3.0f,
            ds::color<f32>{ 0, 0, 0, m_enabled ? 32 : 10 }.nvg(),
            ds::color<f32>{ 0, 0, 0, m_enabled ? 128 : 210 }.nvg()) };

        nvg::begin_path(context);
        nvg::rounded_rect(context, start_x, center.y - 3.0f + 1.0f, width_x, 6.0f, 2.0f);
        nvg::fill_paint(context, bg);
        nvg::fill(context);

        if (m_highlighted_range.second != m_highlighted_range.first)
        {
            nvg::begin_path(context);
            nvg::rounded_rect(context, start_x + m_highlighted_range.first * m_size.width,
                              center.y - kshadow + 1.0f,
                              width_x * (m_highlighted_range.second - m_highlighted_range.first),
                              kshadow * 2.0f, 2.0f);
            nvg::fill_color(context, m_highlight_color.nvg());
            nvg::fill(context);
        }

        nvg::NVGpaint knob_shadow{
            nvg::radial_gradient(context, knob_pos.x, knob_pos.y, kr - kshadow, kr + kshadow,
                                 ds::color<f32>{ 0, 0, 0, 64 }, m_theme->transparent.nvg()),
        };

        nvg::begin_path(context);
        nvg::rect(context, knob_pos.x - kr - 5.0f, knob_pos.y - kr - 5.0f, kr * 2.0f + 10.0f,
                  kr * 2.0f + 10.0f + kshadow);
        nvg::circle(context, knob_pos.x, knob_pos.y, kr);
        nvg::path_winding(context, nvg::NVGHole);
        nvg::fill_paint(context, knob_shadow);
        nvg::fill(context);

        nvg::NVGpaint knob{ nvg::linear_gradient(context, m_pos.x, center.y - kr, m_pos.x,
                                                 center.y + kr, m_theme->border_light.nvg(),
                                                 m_theme->border_medium.nvg()) };
        nvg::NVGpaint knob_reverse{ nvg::linear_gradient(
            context, m_pos.x, center.y - kr, m_pos.x, center.y + kr, m_theme->border_medium.nvg(),
            m_theme->border_light.nvg()) };

        nvg::begin_path(context);
        nvg::circle(context, knob_pos.x, knob_pos.y, kr);
        nvg::stroke_color(context, m_theme->border_dark.nvg());
        nvg::fill_paint(context, knob);
        nvg::stroke(context);
        nvg::fill(context);
        nvg::begin_path(context);
        nvg::circle(context, knob_pos.x, knob_pos.y, kr / 2.0f);
        nvg::fill_color(context, ds::color<f32>{ 150, 150, 150, m_enabled ? 255 : 100 }.nvg());
        nvg::stroke_paint(context, knob_reverse);
        nvg::stroke(context);
        nvg::fill(context);
    }
}
