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

        for (auto child : m_children)
            if (child != nullptr)
                child->release_ref();
    }

    int widget::font_size() const
    {
        return (m_font_size < 0 && m_theme != nullptr)  //
                 ? m_theme->m_standard_font_size        //
                 : m_font_size;                         //
    }

    ds::dims<i32> widget::preferred_size(NVGcontext* nvg_context) const
    {
        return m_layout == nullptr                              //
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
            widget* child{ *it };
            if (child->visible() && child->contains(pt - this->m_pos))
                return child->find_widget({ pt - this->m_pos });
        }
        return this->contains(pt) ? this : nullptr;
    }

    const widget* widget::find_widget(ds::point<i32> pt) const
    {
        for (auto it = m_children.rbegin(); it != m_children.rend(); ++it)
        {
            widget* child = *it;
            if (child->visible() && child->contains(pt - this->m_pos))
                return child->find_widget(pt - this->m_pos);
        }
        return this->contains(pt) ? this : nullptr;
    }

    bool widget::on_kb_character_input(const WindowID id)
    {
        return true;
    }

    bool widget::on_kb_key_pressed(const Keyboard::Button::type key)
    {
        return true;
    }

    bool widget::on_kb_focus_gained(const WindowID id)
    {
        m_focused = true;
        return true;
    }

    bool widget::on_kb_focus_lost(const WindowID id)
    {
        m_focused = false;
        return true;
    }

    bool widget::on_mouse_enter(const WindowID id)
    {
        return true;
    }

    bool widget::on_mouse_leave(const WindowID id)
    {
        return true;
    }

    bool widget::on_mouse_click(const WindowID id)
    {
        return true;
    }

    bool widget::on_mouse_drag(const WindowID id)
    {
        return true;
    }

    bool widget::on_mouse_scroll(const WindowID id)
    {
        return true;
    }

    void widget::add_child(int index, widget* widget)
    {
        runtime_assert(index <= child_count(), "child widget index out of bounds");
        m_children.insert(m_children.begin() + index, widget);
        widget->acquire_ref();
        widget->set_parent(this);
        widget->set_theme(m_theme);
    }

    void widget::add_child(widget* widget)
    {
        add_child(child_count(), widget);
    }

    void widget::remove_child(const widget* widget)
    {
        size_t child_count = m_children.size();
        m_children.erase(std::remove(m_children.begin(), m_children.end(), widget),
                         m_children.end());
        if (m_children.size() == child_count)
            throw std::runtime_error("widget::remove_child(): widget not found!");
        widget->release_ref();
    }

    void widget::remove_child_at(int index)
    {
        if (index < 0 || index >= (int)m_children.size())
            throw std::runtime_error("widget::remove_child_at(): out of bounds!");
        widget* widget = m_children[index];
        m_children.erase(m_children.begin() + index);
        widget->release_ref();
    }

    int widget::child_index(widget* widget) const
    {
        auto it = std::find(m_children.begin(), m_children.end(), widget);
        if (it == m_children.end())
            return -1;
        return (int)(it - m_children.begin());
    }

    rl::Window* widget::window()
    {
        widget* widget{ this };
        while (widget != nullptr)
        {
            rl::Window* window = static_cast<rl::Window*>(widget);
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

        rl::Window* window{ static_cast<rl::Window*>(widget) };
        runtime_assert(window != nullptr, "failed widget cast to window");
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

}
