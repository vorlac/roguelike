#include "core/keyboard.hpp"
#include "core/mouse.hpp"
#include "ds/color.hpp"
#include "ds/dims.hpp"
#include "graphics/vg/nanovg.hpp"
#include "graphics/vg/nanovg_state.hpp"
#include "ui/widget.hpp"
#include "ui/widgets/checkbox.hpp"
#include "utils/unicode.hpp"

namespace rl::ui {
    CheckBox::CheckBox(Widget* parent, std::string caption,
                       const std::function<void(bool)>& toggled_callback)
        : Widget{ parent }
        , m_caption{ std::forward<std::string>(caption) }
        , m_toggled_callback{ toggled_callback }
    {
        // Widget default value override
        m_icon_extra_scale = 1.2f;
    }

    const std::string& CheckBox::caption() const
    {
        return m_caption;
    }

    void CheckBox::set_caption(const std::string& caption)
    {
        m_caption = caption;
    }

    const bool& CheckBox::checked() const
    {
        return m_checked;
    }

    void CheckBox::set_checked(const bool& checked)
    {
        m_checked = checked;
    }

    const bool& CheckBox::pushed() const
    {
        return m_pushed;
    }

    void CheckBox::set_pushed(const bool& pushed)
    {
        m_pushed = pushed;
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

        if (!m_enabled)
            return false;

        if (mouse.is_button_pressed(Mouse::Button::Left)) {
            m_pushed = true;
            return true;
        }

        return false;
    }

    bool CheckBox::on_mouse_button_released(const Mouse& mouse, const Keyboard& kb)
    {
        Widget::on_mouse_button_released(mouse, kb);

        if (!m_enabled)
            return false;
        if (!mouse.is_button_released(Mouse::Button::Left))
            return false;

        if (m_pushed) {
            const ds::point<f32> local_mouse_pos{ mouse.pos() - LocalTransform::absolute_pos };
            if (this->contains(local_mouse_pos)) {
                m_checked = !m_checked;
                if (m_toggled_callback != nullptr)
                    m_toggled_callback(m_checked);
            }

            m_pushed = false;
        }

        return true;
    }

    ds::dims<f32> CheckBox::preferred_size() const
    {
        if (m_fixed_size != ds::dims<f32>::zero())
            return m_fixed_size;

        auto context{ m_renderer->context() };
        nvg::set_font_size(context, this->font_size());
        nvg::set_font_face(context, text::font::style::Sans);

        const f32 text_bounds{ nvg::text_bounds(context, ds::point<f32>::zero(), m_caption) };
        return ds::dims{
            text_bounds + 1.8f * this->font_size(),
            this->font_size() * 1.3f,
        };
    }

    void CheckBox::draw()
    {
        Widget::draw();

        auto context{ m_renderer->context() };
        nvg::set_font_size(context, this->font_size());
        nvg::set_font_face(context, text::font::style::Sans);
        nvg::fill_color(context, m_enabled ? m_theme->text_color : m_theme->disabled_text_color);
        nvg::set_text_align(context, Align::HLeft | Align::VMiddle);
        nvg::draw_text(context,
                       ds::point<f32>{
                           m_rect.pt.x + 1.6f * this->font_size(),
                           m_rect.pt.y + m_rect.size.height * 0.5f,
                       },
                       m_caption);

        const nvg::PaintStyle bg{ nvg::box_gradient(
            context, m_rect.pt.x + 1.5f, m_rect.pt.y + 1.5f, m_rect.size.height - 2.0f,
            m_rect.size.height - 2.0f, 3, 3.0f,
            m_pushed ? ds::color<f32>{ 0, 0, 0, 100 } : ds::color<f32>{ 0, 0, 0, 32 },
            ds::color<f32>{ 0, 0, 0, 180 }) };

        nvg::begin_path(context);
        nvg::rounded_rect(context, m_rect.pt.x + 1.0f, m_rect.pt.y + 1.0f,
                          m_rect.size.height - 2.0f, m_rect.size.height - 2.0f, 3);
        nvg::fill_paint(context, bg);
        nvg::fill(context);

        if (m_checked) {
            nvg::set_font_size(context, this->icon_scale() * m_rect.size.height);
            nvg::set_font_face(context, text::font::style::Icons);
            nvg::fill_color(context, m_enabled ? m_theme->icon_color : m_theme->disabled_text_color);
            nvg::set_text_align(context, Align::HCenter | Align::VMiddle);

            std::string icon_text{ rl::utf8(m_theme->check_box_icon) };
            nvg::draw_text(context,
                           ds::point<f32>{
                               m_rect.pt.x + m_rect.size.height * 0.5f + 1,
                               m_rect.pt.y + m_rect.size.height * 0.5f,
                           },
                           icon_text);
        }
    }
}