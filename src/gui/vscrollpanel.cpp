#include <cmath>

#include "gui/theme.hpp"
#include "gui/vscrollpanel.hpp"

namespace rl::gui {
    VScrollPanel::VScrollPanel(Widget* parent)
        : Widget(parent)
        , m_child_preferred_height(0)
        , m_scroll(0.0f)
    {
    }

    void VScrollPanel::perform_layout(SDL3::SDL_Renderer* ctx)
    {
        Widget::perform_layout(ctx);
        if (m_children.empty())
            return;

        Widget* child = m_children[0];
        m_child_preferred_height = child->preferred_size(ctx).y;
        child->set_relative_position({ 0, 0 });
        child->set_size({ m_size.x - 12, m_child_preferred_height });
    }

    Vector2i VScrollPanel::preferred_size(SDL3::SDL_Renderer* ctx) const
    {
        if (m_children.empty())
            return { 0, 0 };

        return m_children[0]->preferred_size(ctx) + Vector2i(12, 0);
    }

    bool VScrollPanel::mouse_drag_event(const Vector2i&, const Vector2i& rel, int, int)
    {
        if (m_children.empty())
            return false;

        float scrollh = this->height() *
                        std::min(1.0f, this->height() / (float)m_child_preferred_height);

        m_scroll = std::max(0.0f, std::min(1.0f, m_scroll + rel.y / (m_size.y - 8 - scrollh)));
        return true;
    }

    bool VScrollPanel::scroll_event(const Vector2i& /* p */, const Vector2f& rel)
    {
        float scrollAmount = rel.y * (m_size.y / 20.0f);
        float scrollh = height() * std::min(1.0f, height() / (float)m_child_preferred_height);

        m_scroll = std::max(
            (float)0.0f,
            std::min((float)1.0f, m_scroll - scrollAmount / (float)(m_size.y - 8 - scrollh)));
        return true;
    }

    bool VScrollPanel::mouse_button_event(const Vector2i& p, int button, bool down, int modifiers)
    {
        if (m_children.empty())
            return false;
        int shift = (int)(m_scroll * (m_child_preferred_height - m_size.y));
        return m_children[0]->mouse_button_event(p - m_pos + Vector2i{ 0, shift }, button, down,
                                                 modifiers);
    }

    bool VScrollPanel::mouse_motion_event(const Vector2i& p, const Vector2i& rel, int button,
                                          int modifiers)
    {
        if (m_children.empty())
            return false;

        int shift = (int)(m_scroll * (m_child_preferred_height - m_size.y));
        return m_children[0]->mouse_motion_event(p - m_pos + Vector2i{ 0, shift }, rel, button,
                                                 modifiers);
    }

    void VScrollPanel::draw(SDL3::SDL_Renderer* renderer)
    {
        if (m_children.empty())
            return;

        Widget* child = m_children[0];
        m_child_preferred_height = child->preferred_size(nullptr).y;
        float scrollh = height() * std::min(1.0f, height() / (float)m_child_preferred_height);

        SDL3::SDL_Point ap = get_absolute_pos();
        SDL3::SDL_Rect brect{ ap.x, ap.y, width(), height() };

        // SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        // SDL_RenderDrawRect(renderer, &brect);

        if (child->visible())
        {
            const Vector2i savepos = child->relative_position();
            Vector2i npos = savepos;
            m_doffset = -m_scroll * ((float)m_child_preferred_height - (float)m_size.y);
            npos.y += m_doffset;
            child->set_relative_position(npos);
            child->draw(renderer);
            child->set_relative_position(savepos);
        }

        SDL3::SDL_Color sc = m_theme->m_border_dark.sdl_color();
        SDL3::SDL_FRect srect{ ap.x + m_size.x - 12.0f, ap.y + 4.0f, 8.0f, m_size.y - 8.0f };

        SDL3::SDL_SetRenderDrawColor(renderer, sc.r, sc.g, sc.b, sc.a);
        SDL3::SDL_RenderFillRect(renderer, &srect);

        SDL3::SDL_Color ss = m_theme->m_border_light.sdl_color();
        SDL3::SDL_FRect drect{
            std::round(ap.x + m_size.x - 12.0f + 1.0f),
            std::round(ap.y + 4 + (m_size.y - 8.0f - scrollh) * m_scroll + 1.0f),
            6.0f,
            std::round(scrollh - 1.0f),
        };
        SDL3::SDL_SetRenderDrawColor(renderer, ss.r, ss.g, ss.b, ss.a);
        SDL3::SDL_RenderFillRect(renderer, &drect);
    }

    SDL3::SDL_Point VScrollPanel::get_absolute_pos() const
    {
        return Widget::get_absolute_pos();
    }

    PntRect VScrollPanel::get_absolute_cliprect() const
    {
        return Widget::get_absolute_cliprect();
    }

    int VScrollPanel::get_absolute_top() const
    {
        return Widget::get_absolute_top();
    }
}
