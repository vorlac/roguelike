/*
    sdlgui/vscrollpanel.cpp -- Adds a vertical scrollbar around a widget
    that is too big to fit into a certain area

    Based on NanoGUI by Wenzel Jakob <wenzel@inf.ethz.ch>.
    Adaptation for SDL by Dalerank <dalerankn8@gmail.com>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

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

    void VScrollPanel::performLayout(SDL3::SDL_Renderer* ctx)
    {
        Widget::performLayout(ctx);

        if (mChildren.empty())
            return;
        Widget* child = mChildren[0];
        mChildPreferredHeight = child->preferredSize(ctx).y;
        child->setPosition({ 0, 0 });
        child->setSize({ mSize.x - 12, mChildPreferredHeight });
    }

    Vector2i VScrollPanel::preferredSize(SDL3::SDL_Renderer* ctx) const
    {
        if (mChildren.empty())
            return { 0, 0 };
        return mChildren[0]->preferredSize(ctx) + Vector2i(12, 0);
    }

    bool VScrollPanel::mouseDragEvent(const Vector2i&, const Vector2i& rel, int, int)
    {
        if (mChildren.empty())
            return false;

        float scrollh = height() * std::min(1.0f, height() / (float)mChildPreferredHeight);

        mScroll = std::max((float)0.0f,
                           std::min((float)1.0f, mScroll + rel.y / (float)(mSize.y - 8 - scrollh)));
        return true;
    }

    bool VScrollPanel::scrollEvent(const Vector2i& /* p */, const Vector2f& rel)
    {
        float scrollAmount = rel.y * (mSize.y / 20.0f);
        float scrollh = height() * std::min(1.0f, height() / (float)mChildPreferredHeight);

        mScroll = std::max(
            (float)0.0f,
            std::min((float)1.0f, mScroll - scrollAmount / (float)(mSize.y - 8 - scrollh)));
        return true;
    }

    bool VScrollPanel::mouseButtonEvent(const Vector2i& p, int button, bool down, int modifiers)
    {
        if (mChildren.empty())
            return false;
        int shift = (int)(mScroll * (mChildPreferredHeight - mSize.y));
        return mChildren[0]->mouseButtonEvent(p - _pos + Vector2i{ 0, shift }, button, down,
                                              modifiers);
    }

    bool VScrollPanel::mouseMotionEvent(const Vector2i& p, const Vector2i& rel, int button,
                                        int modifiers)
    {
        if (mChildren.empty())
            return false;
        int shift = (int)(mScroll * (mChildPreferredHeight - mSize.y));
        return mChildren[0]->mouseMotionEvent(p - _pos + Vector2i{ 0, shift }, rel, button,
                                              modifiers);
    }

    void VScrollPanel::draw(SDL3::SDL_Renderer* renderer)
    {
        if (mChildren.empty())
            return;

        Widget* child = mChildren[0];
        mChildPreferredHeight = child->preferredSize(nullptr).y;
        float scrollh = height() * std::min(1.0f, height() / (float)mChildPreferredHeight);

        SDL3::SDL_Point ap = getAbsolutePos();
        SDL3::SDL_Rect brect{ ap.x, ap.y, width(), height() };

        // SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        // SDL_RenderDrawRect(renderer, &brect);

        if (child->visible())
        {
            const Vector2i savepos = child->position();
            Vector2i npos = savepos;
            mDOffset = -mScroll * ((float)mChildPreferredHeight - (float)mSize.y);
            npos.y += mDOffset;
            child->setPosition(npos);
            child->draw(renderer);
            child->setPosition(savepos);
        }

        SDL3::SDL_Color sc = m_theme->mBorderDark.toSdlColor();
        SDL3::SDL_FRect srect{ ap.x + mSize.x - 12.0f, ap.y + 4.0f, 8.0f, mSize.y - 8.0f };

        SDL3::SDL_SetRenderDrawColor(renderer, sc.r, sc.g, sc.b, sc.a);
        SDL3::SDL_RenderFillRect(renderer, &srect);

        SDL3::SDL_Color ss = m_theme->mBorderLight.toSdlColor();
        SDL3::SDL_FRect drect{
            std::round(ap.x + mSize.x - 12.0f + 1.0f),
            std::round(ap.y + 4 + (mSize.y - 8.0f - scrollh) * mScroll + 1.0f),
            6.0f,
            std::round(scrollh - 1.0f),
        };
        SDL3::SDL_SetRenderDrawColor(renderer, ss.r, ss.g, ss.b, ss.a);
        SDL3::SDL_RenderFillRect(renderer, &drect);
    }

    SDL3::SDL_Point VScrollPanel::getAbsolutePos() const
    {
        return Widget::getAbsolutePos();
    }

    PntRect VScrollPanel::getAbsoluteCliprect() const
    {
        return Widget::getAbsoluteCliprect();
    }

    int VScrollPanel::getAbsoluteTop() const
    {
        return Widget::getAbsoluteTop();
    }

}
