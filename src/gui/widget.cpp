#include "core/assert.hpp"
#include "gui/layout.hpp"
#include "gui/screen.hpp"
#include "gui/theme.hpp"
#include "gui/widget.hpp"
#include "gui/window.hpp"
#include "sdl/defs.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL.h>
SDL_C_LIB_END

namespace rl::gui {
    Widget::Widget(Widget* parent)
        : m_parent(nullptr)
        , m_theme(nullptr)
        , m_layout(nullptr)
        , m_pos(Vector2i::zero())
        , m_size(Vector2i::zero())
        , m_fixed_size(Vector2i::zero())
        , m_visible(true)
        , m_enabled(true)
        , m_focused(false)
        , m_mouse_focus(false)
        , m_tooltip("")
        , m_font_size(-1.0f)
        , m_cursor(Cursor::Arrow)
    {
        if (parent)
            parent->add_child(this);
    }

    Widget::~Widget()
    {
        for (auto child : m_children)
            if (child)
                child->decRef();
    }

    void Widget::set_theme(Theme* theme)
    {
        if (m_theme.get() == theme)
            return;
        m_theme = theme;
        for (auto child : m_children)
            child->set_theme(theme);
    }

    int Widget::font_size() const
    {
        return (m_font_size < 0 && m_theme) ? m_theme->m_standard_font_size : m_font_size;
    }

    Vector2i Widget::preferred_size(SDL3::SDL_Renderer* ctx) const
    {
        if (m_layout)
            return m_layout->preferred_size(ctx, this);
        else
            return m_size;
    }

    void Widget::perform_layout(SDL3::SDL_Renderer* ctx)
    {
        if (m_layout)
        {
            m_layout->perform_layout(ctx, this);
        }
        else
        {
            for (auto c : m_children)
            {
                Vector2i pref = c->preferred_size(ctx), fix = c->fixed_size();
                c->set_size(Vector2i(fix[0] ? fix[0] : pref[0], fix[1] ? fix[1] : pref[1]));
                c->perform_layout(ctx);
            }
        }
    }

    Widget* Widget::find(const std::string& id, bool inchildren)
    {
        if (m_id == id)
            return this;

        if (inchildren)
        {
            for (auto* child : m_children)
            {
                Widget* w = child->find(id, inchildren);
                if (w)
                    return w;
            }
        }

        return nullptr;
    }

    Widget* Widget::find_widget(const Vector2i& p)
    {
        for (auto it = m_children.rbegin(); it != m_children.rend(); ++it)
        {
            Widget* child = *it;
            if (child->visible() && child->contains(p - m_pos))
                return child->find_widget(p - m_pos);
        }
        return contains(p) ? this : nullptr;
    }

    bool Widget::mouse_button_event(const Vector2i& p, int button, bool down, int modifiers)
    {
        for (auto it = m_children.rbegin(); it != m_children.rend(); ++it)
        {
            Widget* child = *it;
            if (child->visible() && child->contains(p - m_pos) &&
                child->mouse_button_event(p - m_pos, button, down, modifiers))
                return true;
        }

        if (button == SDL_BUTTON_LEFT && down && !m_focused)
            request_focus();
        return false;
    }

    bool Widget::mouse_motion_event(const Vector2i& p, const Vector2i& rel, int button,
                                    int modifiers)
    {
        for (auto it = m_children.rbegin(); it != m_children.rend(); ++it)
        {
            Widget* child = *it;
            if (!child->visible())
                continue;
            bool contained = child->contains(p - m_pos);
            bool prevContained = child->contains(p - m_pos - rel);
            if (contained != prevContained)
                child->mouseEnterEvent(p, contained);
            if ((contained || prevContained) &&
                child->mouse_motion_event(p - m_pos, rel, button, modifiers))
                return true;
        }
        return false;
    }

    bool Widget::scroll_event(const Vector2i& p, const Vector2f& rel)
    {
        for (auto it = m_children.rbegin(); it != m_children.rend(); ++it)
        {
            Widget* child = *it;
            if (!child->visible())
                continue;
            if (child->contains(p - m_pos) && child->scroll_event(p - m_pos, rel))
                return true;
        }
        return false;
    }

    bool Widget::mouse_drag_event(const Vector2i&, const Vector2i&, int, int)
    {
        return false;
    }

    bool Widget::mouseEnterEvent(const Vector2i&, bool enter)
    {
        m_mouse_focus = enter;
        return false;
    }

    bool Widget::focus_event(bool focused)
    {
        m_focused = focused;
        return false;
    }

    bool Widget::kb_button_event(int, int, int, int)
    {
        return false;
    }

    bool Widget::kb_character_event(unsigned int)
    {
        return false;
    }

    void Widget::add_child(size_t index, Widget* widget)
    {
        runtime_assert(index <= child_count(), "widget index already occupied");
        m_children.insert(m_children.begin() + index, widget);
        widget->incRef();
        widget->set_parent(this);
        widget->set_theme(m_theme);
    }

    void Widget::add_child(Widget* widget)
    {
        this->add_child(this->child_count(), widget);
    }

    void Widget::remove_child(const Widget* widget)
    {
        m_children.erase(std::remove(m_children.begin(), m_children.end(), widget),
                         m_children.end());
        widget->decRef();
    }

    void Widget::remove_child(size_t index)
    {
        Widget* widget = m_children[index];
        m_children.erase(m_children.begin() + index);
        widget->decRef();
    }

    int Widget::get_child_index(Widget* widget) const
    {
        auto it = std::find(m_children.begin(), m_children.end(), widget);
        if (it == m_children.end())
            return -1;
        return it - m_children.begin();
    }

    Window* Widget::window()
    {
        Widget* widget = this;
        while (true)
        {
            runtime_assert(widget != nullptr, "widget has no parent window");
            Window* window = dynamic_cast<Window*>(widget);
            if (window)
                return window;
            widget = widget->parent();
        }
    }

    int Widget::get_absolute_left() const
    {
        return m_parent ? m_parent->get_absolute_left() + m_pos.x : m_pos.x;
    }

    SDL3::SDL_Point Widget::get_absolute_pos() const
    {
        if (m_parent == nullptr)
            return SDL3::SDL_Point{ m_pos.x, m_pos.y };
        else
        {
            SDL3::SDL_Point pt{ m_parent->get_absolute_pos() };
            return SDL3::SDL_Point{ pt.x + m_pos.x, pt.y + m_pos.y };
        }
    }

    PntRect Widget::get_absolute_cliprect() const
    {
        if (m_parent)
        {
            PntRect pclip = m_parent->get_absolute_cliprect();
            SDL3::SDL_Point pp = get_absolute_pos();
            PntRect mclip{ pp.x, pp.y, pp.x + width(), pp.y + height() };
            if (pclip.x1 < mclip.x1)
                pclip.x1 = mclip.x1;
            if (pclip.y1 < mclip.y1)
                pclip.y1 = mclip.y1;
            if (mclip.x2 < pclip.x2)
                pclip.x2 = mclip.x2;
            if (mclip.y2 < pclip.y2)
                pclip.y2 = mclip.y2;

            return pclip;
        }
        else
        {
            return PntRect{ m_pos.x, m_pos.y, m_pos.x + width(), m_pos.y + height() };
        }
    }

    int Widget::get_absolute_top() const
    {
        return m_parent ? m_parent->get_absolute_top() + m_pos.y : m_pos.y;
    }

    void Widget::request_focus()
    {
        Widget* widget = this;
        while (widget->parent())
            widget = widget->parent();
        static_cast<Screen*>(widget)->update_focus(this);
    }

    void Widget::draw(SDL3::SDL_Renderer* renderer)
    {
        for (auto&& child : m_children)
            if (child->visible())
                child->draw(renderer);
    }
}
