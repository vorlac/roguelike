#include <ranges>

#include "core/keyboard.hpp"
#include "core/mouse.hpp"
#include "core/ui/dialog.hpp"
#include "core/ui/layout.hpp"
#include "core/ui/screen.hpp"
#include "core/ui/theme.hpp"
#include "core/ui/widget.hpp"
#include "core/window.hpp"
#include "ds/refcounted.hpp"
#include "ds/shared.hpp"
#include "render/vectorized_renderer.hpp"

namespace rl::ui {
    Widget::Widget(Widget* parent)
        : m_parent{ parent }
    {
        if (parent != nullptr)
            parent->add_child(this);
    }

    Widget::Widget(Widget* parent, const std::unique_ptr<VectorizedRenderer>& vec_renderer)
        : m_parent{ parent }
    {
        runtime_assert(m_nvg_renderer == nullptr, "widget vectorized renderer already set");
        if (m_nvg_renderer == nullptr)
            m_nvg_renderer = vec_renderer.get();

        if (parent != nullptr)
            parent->add_child(this);
    }

    Widget::~Widget()
    {
        for (auto child : m_children)
            if (child != nullptr)
                child->release_ref();
    }

    Widget* Widget::parent()
    {
        return m_parent;
    }

    const Widget* Widget::parent() const
    {
        return m_parent;
    }

    void Widget::set_parent(Widget* parent)
    {
        m_parent = parent;
    }

    ui::layout* Widget::layout()
    {
        return m_layout;
    }

    const ui::layout* Widget::layout() const
    {
        return m_layout.get();
    }

    void Widget::set_layout(ui::layout* layout)
    {
        m_layout = layout;
    }

    ui::Theme* Widget::theme()
    {
        return m_theme;
    }

    const ui::Theme* Widget::theme() const
    {
        return m_theme.get();
    }

    void Widget::set_theme(ui::Theme* theme)
    {
        if (m_theme.get() == theme)
            return;

        m_theme = theme;
        for (auto child : m_children)
            child->set_theme(theme);
    }

    ds::point<i32> Widget::position() const
    {
        return m_pos;
    }

    void Widget::set_position(ds::point<i32> pos)
    {
        m_pos = pos;
    }

    ds::point<i32> Widget::abs_position() const
    {
        // Return the position in absolute screen coords
        return m_parent != nullptr                   //
                 ? m_parent->abs_position() + m_pos  //
                 : m_pos;                            //
    }

    ds::dims<i32> Widget::size() const
    {
        return m_size;
    }

    void Widget::set_size(ds::dims<i32> size)
    {
        m_size = size;
    }

    i32 Widget::width() const
    {
        return m_size.width;
    }

    void Widget::set_width(i32 width)
    {
        m_size.width = width;
    }

    i32 Widget::height() const
    {
        return m_size.height;
    }

    void Widget::set_height(i32 height)
    {
        m_size.height = height;
    }

    void Widget::set_fixed_size(ds::dims<i32> fixed_size)
    {
        m_fixed_size = fixed_size;
    }

    ds::dims<i32> Widget::fixed_size() const
    {
        return m_fixed_size;
    }

    i32 Widget::fixed_width() const
    {
        return m_fixed_size.width;
    }

    i32 Widget::fixed_height() const
    {
        return m_fixed_size.height;
    }

    void Widget::set_fixed_width(i32 width)
    {
        m_fixed_size.width = width;
    }

    void Widget::set_fixed_height(i32 height)
    {
        m_fixed_size.height = height;
    }

    bool Widget::visible() const
    {
        return m_visible;
    }

    void Widget::set_visible(bool visible)
    {
        m_visible = visible;
    }

    bool Widget::show()
    {
        this->set_visible(true);
        return true;
    }

    bool Widget::hide()
    {
        this->set_visible(false);
        return true;
    }

    bool Widget::visible_recursive() const
    {
        bool visible{ true };

        const Widget* widget{ this };
        while (widget != nullptr)
        {
            visible &= widget->visible();
            widget = widget->parent();
        }

        return visible;
    }

    i32 Widget::child_count() const
    {
        return static_cast<i32>(m_children.size());
    }

    const Widget* Widget::child_at(i32 index) const
    {
        return m_children[index];
    }

    Widget* Widget::child_at(i32 index)
    {
        return m_children[index];
    }

    const std::vector<Widget*>& Widget::children() const
    {
        return m_children;
    }

    int Widget::font_size() const
    {
        return (m_font_size < 0 && m_theme != nullptr)  //
                 ? m_theme->m_standard_font_size        //
                 : m_font_size;                         //
    }

    ds::dims<i32> Widget::preferred_size(NVGcontext* nvg_context) const
    {
        return m_layout != nullptr                              //
                 ? m_layout->preferred_size(nvg_context, this)  //
                 : m_size;                                      //
    }

    void Widget::perform_layout(NVGcontext* nvg_context)
    {
        if (m_layout != nullptr)
            m_layout->perform_layout(nvg_context, this);
        else
        {
            for (auto child : m_children)
            {
                ds::dims<i32>&& pref{ child->preferred_size(nvg_context) };
                ds::dims<i32>&& fix{ child->fixed_size() };
                child->set_size(ds::dims<i32>{
                    fix.width ? fix.width : pref.width,
                    fix.height ? fix.height : pref.height,
                });
                child->perform_layout(nvg_context);
            }
        }
    }

    Widget* Widget::find_widget(ds::point<i32> pt)
    {
        for (auto child : std::ranges::reverse_view{ m_children })
            if (child->visible() && child->contains(pt))
                return child->find_widget(pt);

        return this->contains(pt) ? this : nullptr;
    }

    const Widget* Widget::find_widget(ds::point<i32> pt) const
    {
        for (auto child : std::ranges::reverse_view{ m_children })
            if (child->visible() && child->contains(pt))
                return child->find_widget(pt);

        return this->contains(pt) ? this : nullptr;
    }

    bool Widget::on_mouse_entered(const Mouse& mouse)
    {
        m_mouse_focus = true;
        return false;
    }

    bool Widget::on_mouse_exited(const Mouse& mouse)
    {
        m_mouse_focus = false;
        return false;
    }

    bool Widget::on_focus_gained()
    {
        m_focused = true;
        return true;
    }

    bool Widget::on_focus_lost()
    {
        m_focused = false;
        return true;
    }

    bool Widget::on_mouse_button_pressed(const Mouse& mouse, const Keyboard& kb)
    {
        auto&& mouse_pos{ mouse.pos() };
        for (auto child : std::ranges::reverse_view{ m_children })
            if (child->visible() && child->contains(mouse_pos) &&
                child->on_mouse_button_pressed(mouse, kb))
                return true;

        if (mouse.is_button_pressed(Mouse::Button::Left) && !m_focused)
            this->request_focus();

        return false;
    }

    bool Widget::on_mouse_button_released(const Mouse& mouse, const Keyboard& kb)
    {
        auto&& mouse_pos{ mouse.pos() };
        for (auto child : std::ranges::reverse_view{ m_children })
            if (child->visible() && child->contains(mouse_pos) &&
                child->on_mouse_button_released(mouse, kb))
                return true;

        return false;
    }

    bool Widget::on_mouse_scroll(const Mouse& mouse, const Keyboard& kb)
    {
        for (auto child : std::ranges::reverse_view{ m_children })
        {
            if (!child->visible())
                continue;

            auto&& mouse_pos{ mouse.pos() };
            if (child->contains(mouse_pos) && child->on_mouse_scroll(mouse, kb))
                return true;
        }
        return false;
    }

    bool Widget::on_mouse_move(const Mouse& mouse, const Keyboard& kb)
    {
        bool handled{ false };

        for (auto child : std::ranges::reverse_view{ m_children })
        {
            if (!child->visible())
                continue;

            const auto mouse_pos{ mouse.pos() };
            const bool contained{ child->contains(mouse_pos) };
            const bool prev_contained{ child->contains(mouse_pos - mouse.pos_delta()) };

            if (contained && !prev_contained)
                handled |= child->on_mouse_entered(mouse);
            else if (!contained && prev_contained)
                handled |= child->on_mouse_exited(mouse);

            if (contained || prev_contained)
                handled |= child->on_mouse_move(mouse, kb);
        }

        return handled;
    }

    bool Widget::on_mouse_drag(const Mouse& mouse, const Keyboard& kb)
    {
        // do nothing,
        // derived objects should implement
        return false;
    }

    bool Widget::on_key_pressed(const Keyboard& kb)
    {
        // do nothing,
        // derived objects should implement
        return false;
    }

    bool Widget::on_key_released(const Keyboard& kb)
    {
        // do nothing,
        // derived objects should implement
        return false;
    }

    bool Widget::on_character_input(const Keyboard& kb)
    {
        // do nothing,
        // derived objects should implement
        return false;
    }

    void Widget::add_child(i32 index, Widget* widget)
    {
        runtime_assert(index <= child_count(), "child widget index out of bounds");
        m_children.insert(m_children.begin() + index, widget);

        widget->acquire_ref();
        widget->set_parent(this);
        widget->set_theme(m_theme);
    }

    void Widget::add_child(ui::Widget* widget)
    {
        this->add_child(this->child_count(), widget);
    }

    void Widget::remove_child(const ui::Widget* widget)
    {
        size_t child_count{ m_children.size() };
        m_children.erase(std::remove(m_children.begin(), m_children.end(), widget),
                         m_children.end());

        runtime_assert(m_children.size() != child_count, "didn't find widget to delete");
        widget->release_ref();
    }

    void Widget::remove_child_at(i32 index)
    {
        runtime_assert(index > 0 && index < m_children.size(),
                       "widget child remove idx out of bounds");

        ui::Widget* widget{ m_children[index] };
        m_children.erase(m_children.begin() + index);
        widget->release_ref();
    }

    i32 Widget::child_index(ui::Widget* widget) const
    {
        auto it = std::find(m_children.begin(), m_children.end(), widget);
        if (it == m_children.end())
            return -1;

        return static_cast<i32>(it - m_children.begin());
    }

    bool Widget::enabled() const
    {
        return m_enabled;
    }

    void Widget::set_enabled(bool enabled)
    {
        m_enabled = enabled;
    }

    bool Widget::focused() const
    {
        return m_focused;
    }

    void Widget::set_focused(bool focused)
    {
        m_focused = focused;
    }

    const std::string& Widget::tooltip() const
    {
        return m_tooltip;
    }

    void Widget::set_tooltip(const std::string& tooltip)
    {
        m_tooltip = tooltip;
    }

    void Widget::set_font_size(i32 font_size)
    {
        m_font_size = font_size;
    }

    bool Widget::has_font_size() const
    {
        return m_font_size > 0;
    }

    f32 Widget::icon_extra_scale() const
    {
        return m_icon_extra_scale;
    }

    void Widget::set_icon_extra_scale(f32 scale)
    {
        m_icon_extra_scale = scale;
    }

    Mouse::Cursor::ID Widget::cursor() const
    {
        return m_cursor;
    }

    void Widget::set_cursor(Mouse::Cursor::ID cursor)
    {
        m_cursor = cursor;
    }

    bool Widget::contains(ds::point<i32> pt) const
    {
        // Check if the widget contains a certain position
        ds::rect<i32> widget_rect{ this->abs_position(), m_size };
        return widget_rect.contains(pt);
    }

    ui::Dialog* Widget::dialog()
    {
        ui::Widget* widget{ this };
        while (widget != nullptr)
        {
            ui::Dialog* dialog{ dynamic_cast<ui::Dialog*>(widget) };
            runtime_assert(dialog != nullptr, "failed widget cast to window");

            if (dialog != nullptr)
                return dialog;

            widget = widget->parent();
        }

        runtime_assert(false, "failed to get window that owns widget");
        return nullptr;
    }

    const ui::Dialog* Widget::dialog() const
    {
        return const_cast<ui::Widget*>(this)->dialog();
    }

    void Widget::request_focus()
    {
        ui::Widget* widget{ this };
        while (widget->parent() != nullptr)
            widget = widget->parent();

        static_cast<ui::Screen*>(widget)->update_focus(this);
    }

    void Widget::draw_mouse_intersection(NVGcontext* nvg_context, ds::point<i32> pt)
    {
        // if constexpr (Widget::DiagnosticsEnabled)
        //{
        if (!this->contains(pt))
            return;

        //  render green widget outlines
        ds::rect<i32>{ this->abs_position(), m_size };
        m_nvg_renderer->draw_rect_outline(ds::rect<i32>{ this->abs_position(), m_size }, 1.0f,
                                          rl::Colors::Cyan, ui::Outline::Inner);

        for (auto child : m_children)
        {
            if (!child->visible())
                continue;

            child->draw_mouse_intersection(nvg_context, pt);
        }
        //}
    }

    void Widget::draw(NVGcontext* nvg_context)
    {
        if constexpr (Widget::DiagnosticsEnabled)
        {
            m_nvg_renderer->draw_rect_outline(ds::rect<i32>{ this->abs_position(), m_size }, 1.0f,
                                              rl::Colors::Grey, ui::Outline::Outer);
        }

        if (m_children.empty())
            return;

        nvgTranslate(nvg_context, m_pos.x, m_pos.y);

        for (auto child : m_children)
        {
            if (!child->visible())
                continue;

            if constexpr (!Widget::DiagnosticsEnabled)
            {
                m_nvg_renderer->push_render_state();
                nvgIntersectScissor(nvg_context, child->m_pos.x, child->m_pos.y,
                                    child->m_size.width, child->m_size.height);
            }

            child->draw(nvg_context);

            if constexpr (!Widget::DiagnosticsEnabled)
                m_nvg_renderer->pop_render_state();
        }

        nvgTranslate(nvg_context, -m_pos.x, -m_pos.y);
    }

    f32 Widget::icon_scale() const
    {
        runtime_assert(m_theme != nullptr, "theme not set");
        return m_theme->m_icon_scale * m_icon_extra_scale;
    }
}
