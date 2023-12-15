#include <cmath>

#include "gui/theme.hpp"
#include "gui/vscrollpanel.hpp"

namespace rl::gui {
    VScrollPanel::VScrollPanel(Widget* parent)
        : Widget(parent)
        , mChildPreferredHeight(0)
        , mScroll(0.0f)
    {
    }

    void VScrollPanel::perform_layout(SDL3::SDL_Renderer* ctx)
    {
        Widget::perform_layout(ctx);

        if (m_children.empty())
            return;
        Widget* child = m_children[0];
        mChildPreferredHeight = child->preferredSize(ctx).y;
        child->set_relative_position({ 0, 0 });
        child->set_size({ m_size.x - 12, mChildPreferredHeight });
    }

    Vector2i VScrollPanel::preferredSize(SDL3::SDL_Renderer* ctx) const
    {
        if (m_children.empty())
            return { 0, 0 };
        return m_children[0]->preferredSize(ctx) + Vector2i(12, 0);
    }

    bool VScrollPanel::mouseDragEvent(const Vector2i&, const Vector2i& rel, int, int)
    {
        if (m_children.empty())
            return false;

        float scrollh = height() * std::min(1.0f, height() / (float)mChildPreferredHeight);

        mScroll = std::max(
            (float)0.0f, std::min((float)1.0f, mScroll + rel.y / (float)(m_size.y - 8 - scrollh)));
        return true;
    }

    bool VScrollPanel::scrollEvent(const Vector2i& /* p */, const Vector2f& rel)
    {
        float scrollAmount = rel.y * (m_size.y / 20.0f);
        float scrollh = height() * std::min(1.0f, height() / (float)mChildPreferredHeight);

        mScroll = std::max(
            (float)0.0f,
            std::min((float)1.0f, mScroll - scrollAmount / (float)(m_size.y - 8 - scrollh)));
        return true;
    }

    bool VScrollPanel::mouseButtonEvent(const Vector2i& p, int button, bool down, int modifiers)
    {
        if (m_children.empty())
            return false;
        int shift = (int)(mScroll * (mChildPreferredHeight - m_size.y));
        return m_children[0]->mouseButtonEvent(p - m_pos + Vector2i{ 0, shift }, button, down,
                                               modifiers);
    }

    bool VScrollPanel::mouseMotionEvent(const Vector2i& p, const Vector2i& rel, int button,
                                        int modifiers)
    {
        if (m_children.empty())
            return false;
        int shift = (int)(mScroll * (mChildPreferredHeight - m_size.y));
        return m_children[0]->mouseMotionEvent(p - m_pos + Vector2i{ 0, shift }, rel, button,
                                               modifiers);
    }

    void VScrollPanel::draw(SDL3::SDL_Renderer* renderer)
    {
        if (m_children.empty())
            return;

        Widget* child = m_children[0];
        mChildPreferredHeight = child->preferredSize(nullptr).y;
        float scrollh = height() * std::min(1.0f, height() / (float)mChildPreferredHeight);

        SDL3::SDL_Point ap = get_absolute_pos();
        SDL3::SDL_Rect brect{ ap.x, ap.y, width(), height() };

        // SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        // SDL_RenderDrawRect(renderer, &brect);

        if (child->visible())
        {
            const Vector2i savepos = child->relative_position();
            Vector2i npos = savepos;
            mDOffset = -mScroll * ((float)mChildPreferredHeight - (float)m_size.y);
            npos.y += mDOffset;
            child->set_relative_position(npos);
            child->draw(renderer);
            child->set_relative_position(savepos);
        }

        SDL3::SDL_Color sc = m_theme->mBorderDark.toSdlColor();
        SDL3::SDL_FRect srect{ ap.x + m_size.x - 12.0f, ap.y + 4.0f, 8.0f, m_size.y - 8.0f };

        SDL3::SDL_SetRenderDrawColor(renderer, sc.r, sc.g, sc.b, sc.a);
        SDL3::SDL_RenderFillRect(renderer, &srect);

        SDL3::SDL_Color ss = m_theme->mBorderLight.toSdlColor();
        SDL3::SDL_FRect drect{
            std::round(ap.x + m_size.x - 12.0f + 1.0f),
            std::round(ap.y + 4 + (m_size.y - 8.0f - scrollh) * mScroll + 1.0f),
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
