#include "core/keyboard.hpp"
#include "core/mouse.hpp"
#include "ds/color.hpp"
#include "ds/dims.hpp"
#include "gfx/vg/nanovg.hpp"
#include "gfx/vg/nanovg_state.hpp"
#include "ui/widget.hpp"
#include "ui/widgets/checkbox.hpp"
#include "utils/unicode.hpp"

namespace rl::ui {
    CheckBox::CheckBox(std::string text, const std::function<void(bool)>& toggled_callback)
        : CheckBox{ nullptr, std::move(text), toggled_callback }
    {
    }

    CheckBox::CheckBox(Widget* parent, std::string text, const std::function<void(bool)>& toggled_callback)
        : Widget{ parent }
        , m_text{ std::move(text) }
        , m_toggled_callback{ toggled_callback }
    {
        // TODO: move into stylesheet
        // scale checkmark to 80% so it
        // cleanly fits in the square
        m_icon_extra_scale = 0.75f;
    }

    std::string_view CheckBox::text() const
    {
        return m_text;
    }

    bool CheckBox::checked() const
    {
        return m_checked;
    }

    bool CheckBox::pressed() const
    {
        return m_pressed;
    }

    void CheckBox::set_text(std::string text)
    {
        m_text = std::move(text);
    }

    void CheckBox::set_checked(const bool checked)
    {
        m_checked = checked;
    }

    void CheckBox::set_pressed(const bool pressed)
    {
        m_pressed = pressed;
    }

    const std::function<void(bool)>& CheckBox::callback() const
    {
        return m_toggled_callback;
    }

    void CheckBox::set_callback(const std::function<void(bool)>& toggled_callback)
    {
        m_toggled_callback = toggled_callback;
    }

    bool CheckBox::on_mouse_button_pressed(const Mouse& mouse, const Keyboard& kb)
    {
        Widget::on_mouse_button_pressed(mouse, kb);
        if (m_enabled && mouse.is_button_pressed(Mouse::Button::Left)) {
            m_pressed = true;
            return true;
        }

        return false;
    }

    bool CheckBox::on_mouse_button_released(const Mouse& mouse, const Keyboard& kb)
    {
        Widget::on_mouse_button_released(mouse, kb);

        if (!m_enabled)
            return false;

        if (m_pressed && mouse.is_button_released(Mouse::Button::Left)) {
            const ds::point<f32> local_mouse_pos{ mouse.pos() - LocalTransform::absolute_pos };
            if (this->contains(local_mouse_pos)) {
                m_checked = !m_checked;
                if (m_toggled_callback != nullptr)
                    m_toggled_callback(m_checked);
            }

            m_pressed = false;
        }

        return true;
    }

    ds::dims<f32> CheckBox::preferred_size() const
    {
        if (!m_fixed_size.is_empty())
            return m_fixed_size;

        const auto context{ m_renderer->context() };
        const f32 font_size{ m_theme->check_box_font_size };
        nvg::set_font_size(context, font_size);
        nvg::set_font_face(context, m_theme->checkbox_text_font);

        const f32 text_width{ nvg::text_bounds(context, ds::point<f32>::zero(), m_text) };
        const ds::dims pref_size{
            text_width + (font_size * (1.0f + TEXT_OFFSET)),
            font_size,
        };

        return pref_size;
    }

    void CheckBox::draw()
    {
        Widget::draw();

        const auto context{ m_renderer->context() };
        const f32 checkbox_height{
            math::equal(m_fixed_size.height, 0.0f)
                ? m_theme->check_box_font_size
                : m_fixed_size.height
        };

        const TextProperties props{
            .font = m_theme->checkbox_text_font,
            .align = Align::VMiddle | Align::HLeft,
            .color = m_enabled ? m_theme->text_color
                               : m_theme->disabled_text_color,
            .size = checkbox_height,
        };
        const ds::rect<f32> checkbox_rect{
            ds::point{
                m_rect.pt.x,
                m_rect.pt.y +
                    m_rect.size.height / 2.0f -
                    checkbox_height / 2.0f,
            },
            ds::dims{
                checkbox_height,
                checkbox_height,
            },
        };

        // start text horixontally adjacent to cb square
        // and center text vertically within the widget
        const ds::point<f32> text_pos{
            m_rect.pt.x + (checkbox_height * (1.0f + TEXT_OFFSET)),
            m_rect.pt.y + (m_rect.size.height / 2.0f)
        };

        m_renderer->draw_text(m_text, text_pos, props);

        const nvg::PaintStyle bg{
            m_renderer->create_rect_gradient_paint_style(
                checkbox_rect, CORNER_RADIUS, OUTER_BLUR,
                m_pressed ? ds::color<f32>{ 0, 0, 0, 100 }
                          : ds::color<f32>{ 0, 0, 0, 32 },
                ds::color<f32>{ 0, 0, 0, 180 })
        };

        // draw the sunken checkbox square
        // -2 pixels in size so it's fully
        // contained by the widget's rect
        m_renderer->draw_path(false, [&] {
            m_renderer->draw_rounded_rect(
                checkbox_rect.expanded(-2.0f),
                CORNER_RADIUS);
            nvg::fill_paint(context, bg);
            nvg::fill(context);
        });

        if (m_checked) {
            // draw the check mark
            const f32 icon_scale{ m_icon_extra_scale * m_theme->icon_scale };
            nvg::set_font_face(context, text::font::style::Icons);
            nvg::set_font_size(context, checkbox_height * icon_scale);
            nvg::fill_color(context, m_enabled ? m_theme->icon_color : m_theme->disabled_text_color);
            nvg::set_text_align(context, Align::HCenter | Align::VMiddle);
            nvg::draw_text(
                context,
                ds::point<f32>{
                    m_rect.pt.x + checkbox_height * 0.5f,
                    m_rect.pt.y + m_rect.size.height * 0.5f,
                },
                utf8(m_theme->check_box_icon));
        }
    }
}
