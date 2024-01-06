#include "core/keyboard.hpp"
#include "core/mouse.hpp"
#include "core/ui/layout.hpp"
#include "core/ui/theme.hpp"
#include "core/ui/widget.hpp"
#include "core/window.hpp"
#include "ds/refcounted.hpp"
#include "ds/shared.hpp"

namespace rl::ui {

    widget::widget(widget* parent)
        : m_parent{ nullptr }
        , m_theme{ nullptr }
        , m_layout{ nullptr }
        , m_pos{ 0, 0 }
        , m_size{ 0, 0 }
        , m_fixed_size{ 0, 0 }
        , m_visible{ true }
        , m_enabled{ true }
        , m_focused{ false }
        , m_mouse_focus{ false }
        , m_tooltip{}
        , m_font_size{ 16 }
        , m_icon_extra_scale{ 1.0f }
        , m_cursor{ Mouse::Cursor::ID::Arrow }
    {
        if (parent != nullptr)
            parent->add_child(this);
    }

    widget::~widget()
    {
        if (std::uncaught_exceptions() > 0)
        {
            // If a widget constructor throws an exception, it is immediately
            // dealloated but may still be referenced by a parent. Be conservative
            // and don't decrease the reference count of children while dispatching
            // exceptions.
            return;
        }

        for (auto&& child : m_children)
            if (child != nullptr)
                child->release_ref();
    }

    widget* widget::parent()
    {
        return m_parent;
    }

    const widget* widget::parent() const
    {
        return m_parent;
    }

    void widget::set_parent(widget* parent)
    {
        m_parent = parent;
    }

    ui::layout* widget::layout()
    {
        return m_layout;
    }

    const ui::layout* widget::layout() const
    {
        return m_layout.get();
    }

    void widget::set_layout(ui::layout* layout)
    {
        m_layout = layout;
    }

    ui::theme* widget::theme()
    {
        return m_theme;
    }

    const ui::theme* widget::theme() const
    {
        return m_theme.get();
    }

    void widget::set_theme(ui::theme* theme)
    {
        if (m_theme.get() == theme)
            return;

        m_theme = theme;
        for (auto&& child : m_children)
            child->set_theme(theme);
    }

    ds::point<i32> widget::position() const
    {
        return m_pos;
    }

    void widget::set_position(ds::point<i32> pos)
    {
        m_pos = pos;
    }

    ds::point<i32> widget::abs_position() const
    {
        // Return the position in absolute screen coords
        return m_parent != nullptr                   //
                 ? m_parent->abs_position() + m_pos  //
                 : m_pos;                            //
    }

    ds::dims<i32> widget::size() const
    {
        return m_size;
    }

    void widget::set_size(const ds::dims<i32>& size)
    {
        m_size = size;
    }

    i32 widget::width() const
    {
        return m_size.width;
    }

    void widget::set_width(i32 width)
    {
        m_size.width = width;
    }

    i32 widget::height() const
    {
        return m_size.height;
    }

    void widget::set_height(i32 height)
    {
        m_size.height = height;
    }

    void widget::set_fixed_size(ds::dims<i32> fixed_size)
    {
        m_fixed_size = fixed_size;
    }

    ds::dims<i32> widget::fixed_size() const
    {
        return m_fixed_size;
    }

    i32 widget::fixed_width() const
    {
        return m_fixed_size.width;
    }

    i32 widget::fixed_height() const
    {
        return m_fixed_size.height;
    }

    void widget::set_fixed_width(i32 width)
    {
        m_fixed_size.width = width;
    }

    void widget::set_fixed_height(i32 height)
    {
        m_fixed_size.height = height;
    }

    bool widget::visible() const
    {
        return m_visible;
    }

    void widget::set_visible(bool visible)
    {
        m_visible = visible;
    }

    bool widget::show()
    {
        this->set_visible(true);
        return true;
    }

    bool widget::hide()
    {
        this->set_visible(false);
        return true;
    }

    bool widget::visible_recursive() const
    {
        bool visible{ true };

        const widget* widget{ this };
        while (widget != nullptr)
        {
            visible &= widget->visible();
            widget = widget->parent();
        }

        return visible;
    }

    i32 widget::child_count() const
    {
        return static_cast<i32>(m_children.size());
    }

    const widget* widget::child_at(i32 index) const
    {
        return m_children[index];
    }

    widget* widget::child_at(i32 index)
    {
        return m_children[index];
    }

    const std::vector<widget*>& widget::children() const
    {
        return m_children;
    }

    int widget::font_size() const
    {
        return (m_font_size < 0 && m_theme != nullptr)  //
                 ? m_theme->m_standard_font_size        //
                 : m_font_size;                         //
    }

    ds::dims<i32> widget::preferred_size(NVGcontext* nvg_context) const
    {
        return m_layout != nullptr                              //
                 ? m_layout->preferred_size(nvg_context, this)  //
                 : m_size;                                      //
    }

    void widget::perform_layout(NVGcontext* nvg_context)
    {
        if (m_layout != nullptr)
            m_layout->perform_layout(nvg_context, this);
        else
        {
            for (auto&& c : m_children)
            {
                ds::dims<i32>&& pref{ c->preferred_size(nvg_context) };
                ds::dims<i32>&& fix{ c->fixed_size() };
                c->set_size(ds::dims<i32>{
                    fix.width ? fix.width : pref.width,
                    fix.height ? fix.height : pref.height,
                });
                c->perform_layout(nvg_context);
            }
        }
    }

    widget* widget::find_widget(ds::point<i32> pt)
    {
        for (auto it = m_children.rbegin(); it != m_children.rend(); ++it)
        {
            ui::widget* child{ *it };
            if (child->visible() && child->contains(pt - this->m_pos))
                return child->find_widget({ pt - this->m_pos });
        }
        return this->contains(pt) ? this : nullptr;
    }

    const widget* widget::find_widget(ds::point<i32> pt) const
    {
        for (auto it = m_children.rbegin(); it != m_children.rend(); ++it)
        {
            ui::widget* child{ *it };
            if (child->visible() && child->contains(pt - this->m_pos))
                return child->find_widget(pt - this->m_pos);
        }
        return this->contains(pt) ? this : nullptr;
    }

    bool widget::on_mouse_entered(const Mouse& mouse)
    {
        m_mouse_focus = true;
        return false;
    }

    bool widget::on_mouse_exited(const Mouse& mouse)
    {
        m_mouse_focus = false;
        return false;
    }

    bool widget::on_focus_gained()
    {
        m_focused = true;
        return true;
    }

    bool widget::on_focus_lost()
    {
        m_focused = false;
        return true;
    }

    bool widget::on_mouse_button_pressed(const Mouse& mouse, const Keyboard& kb)
    {
        auto&& mouse_pos{ mouse.pos() };
        for (auto it = m_children.rbegin(); it != m_children.rend(); it++)
        {
            ui::widget* child{ *it };
            if (child->visible() && child->contains(mouse_pos) &&
                child->on_mouse_button_pressed(mouse, kb))
                return true;
        }

        if (mouse.is_button_pressed(Mouse::Button::Left) && !m_focused)
            this->request_focus();

        return false;
    }

    bool widget::on_mouse_button_released(const Mouse& mouse, const Keyboard& kb)
    {
        auto&& mouse_pos{ mouse.pos() };
        for (auto it = m_children.rbegin(); it != m_children.rend(); ++it)
        {
            ui::widget* child{ *it };
            auto&& offset_pos{ mouse_pos - m_pos };
            if (child->visible() && child->contains(mouse_pos) &&
                child->on_mouse_button_released(mouse, kb))
                return true;
        }

        return false;
    }

    bool widget::on_mouse_scroll(const Mouse& mouse, const Keyboard& kb)
    {
        for (auto it = m_children.rbegin(); it != m_children.rend(); ++it)
        {
            ui::widget* child{ *it };
            if (!child->visible())
                continue;

            auto&& mouse_pos{ mouse.pos() };
            auto&& offset_mouse_pos{ mouse_pos - m_pos };
            if (child->contains(offset_mouse_pos) && child->on_mouse_scroll(mouse, kb))
                return true;
        }
        return false;
    }

    bool widget::on_mouse_move(const Mouse& mouse, const Keyboard& kb)
    {
        bool handled{ false };

        for (auto&& child : m_children)
        {
            if (!child->visible())
                continue;

            auto&& mouse_pos{ mouse.pos() };
            auto&& mouse_delta_pos{ mouse.pos_delta() };

            const bool contained{ child->contains(mouse_pos - m_pos) };
            const bool prev_contained{ child->contains(mouse_pos - m_pos - mouse_delta_pos) };

            if (contained != prev_contained)
                handled |= child->on_mouse_entered(mouse);

            if (contained || prev_contained)
                handled |= child->on_mouse_move(mouse, kb);
        }

        return handled;
    }

    bool widget::on_mouse_drag(ds::point<i32> pnt, ds::vector2<i32> rel, const Mouse& mouse,
                               const Keyboard& kb)
    {
        // do nothing,
        // derived objects should implement
        return false;
    }

    bool widget::on_key_pressed(const Keyboard& kb)
    {
        // do nothing,
        // derived objects should implement
        return false;
    }

    bool widget::on_key_released(const Keyboard& kb)
    {
        // do nothing,
        // derived objects should implement
        return false;
    }

    bool widget::on_character_input(const Keyboard& kb)
    {
        // do nothing,
        // derived objects should implement
        return false;
    }

    void widget::add_child(i32 index, widget* widget)
    {
        runtime_assert(index <= child_count(), "child widget index out of bounds");
        m_children.insert(m_children.begin() + index, widget);
        widget->acquire_ref();
        widget->set_parent(this);
        widget->set_theme(m_theme);
    }

    void widget::add_child(widget* widget)
    {
        this->add_child(this->child_count(), widget);
    }

    void widget::remove_child(const widget* widget)
    {
        size_t child_count{ m_children.size() };
        m_children.erase(std::remove(m_children.begin(), m_children.end(), widget),
                         m_children.end());

        runtime_assert(m_children.size() != child_count, "didn't find widget to delete");
        widget->release_ref();
    }

    void widget::remove_child_at(i32 index)
    {
        runtime_assert(index > 0 && index < m_children.size(),
                       "widget child remove idx out of bounds");

        widget* widget{ m_children[index] };
        m_children.erase(m_children.begin() + index);
        widget->release_ref();
    }

    i32 widget::child_index(widget* widget) const
    {
        auto it = std::find(m_children.begin(), m_children.end(), widget);
        if (it == m_children.end())
            return -1;

        return static_cast<i32>(it - m_children.begin());
    }

    bool widget::enabled() const
    {
        return m_enabled;
    }

    void widget::set_enabled(bool enabled)
    {
        m_enabled = enabled;
    }

    bool widget::focused() const
    {
        return m_focused;
    }

    void widget::set_focused(bool focused)
    {
        m_focused = focused;
    }

    const std::string& widget::tooltip() const
    {
        return m_tooltip;
    }

    void widget::set_tooltip(const std::string& tooltip)
    {
        m_tooltip = tooltip;
    }

    void widget::set_font_size(int font_size)
    {
        m_font_size = font_size;
    }

    bool widget::has_font_size() const
    {
        return m_font_size > 0;
    }

    float widget::icon_extra_scale() const
    {
        return m_icon_extra_scale;
    }

    void widget::set_icon_extra_scale(float scale)
    {
        m_icon_extra_scale = scale;
    }

    Mouse::Cursor::ID widget::cursor() const
    {
        return m_cursor;
    }

    void widget::set_cursor(Mouse::Cursor::ID cursor)
    {
        m_cursor = cursor;
    }

    bool widget::contains(ds::point<i32> pt) const
    {
        // Check if the widget contains a certain position
        ds::rect<i32> widget_rect{ this->abs_position(), m_size };
        return widget_rect.contains(pt);
    }

    Window* widget::window()
    {
        widget* widget{ this };
        while (widget != nullptr)
        {
            Window* window{ dynamic_cast<Window*>(widget) };
            runtime_assert(window != nullptr, "failed widget cast to window");
            if (window != nullptr)
                return window;

            widget = widget->parent();
        }

        runtime_assert(false, "failed to get window that owns widget");
        return nullptr;
    }

    const Window* widget::window() const
    {
        return const_cast<widget*>(this)->window();
    }

    void widget::request_focus()
    {
        widget* widget{ this };
        while (widget->parent() != nullptr)
            widget = widget->parent();

        Window* window{ dynamic_cast<Window*>(widget) };
        runtime_assert(window != nullptr, "failed widget conversion to window");
        window->update_focus(this);
    }

    void widget::draw(NVGcontext* nvg_context)
    {
        if constexpr (widget::DiagnosticsEnabled)
        {
            // render red widget outlines
            nvgStrokeWidth(nvg_context, 1.0f);
            nvgBeginPath(nvg_context);
            nvgRect(nvg_context, this->m_pos.x - 0.5f, this->m_pos.y - 0.5f, m_size.width + 1,
                    m_size.height + 1);
            nvgStrokeColor(nvg_context, nvgRGBA(255, 0, 0, 255));
            nvgStroke(nvg_context);
        }

        if (m_children.empty())
            return;

        nvgTranslate(nvg_context, this->m_pos.x, this->m_pos.y);
        for (auto child : m_children)
        {
            if (!child->visible())
                continue;

            if constexpr (!widget::DiagnosticsEnabled)
            {
                nvgSave(nvg_context);
                nvgIntersectScissor(nvg_context, child->m_pos.x, child->m_pos.y,
                                    child->m_size.width, child->m_size.height);
            }

            child->draw(nvg_context);

            if constexpr (!widget::DiagnosticsEnabled)
                nvgRestore(nvg_context);
        }
        nvgTranslate(nvg_context, -this->m_pos.x, -this->m_pos.y);
    }

    float widget::icon_scale() const
    {
        return m_theme->m_icon_scale * m_icon_extra_scale;
    }
}
