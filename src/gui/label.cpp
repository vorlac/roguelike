#include "gui/label.hpp"
#include "gui/theme.hpp"

namespace rl::gui {
    Label::Label(Widget* parent, const std::string& caption, const std::string& font, int fontSize)
        : Widget(parent)
        , m_caption(caption)
        , mFont(font)
    {
        if (m_theme)
        {
            m_font_size = m_theme->mStandardFontSize;
            mColor = m_theme->mTextColor;
        }

        if (fontSize >= 0)
            m_font_size = fontSize;

        _texture.dirty = true;
    }

    void Label::set_theme(Theme* theme)
    {
        Widget::set_theme(theme);
        if (m_theme)
        {
            m_font_size = m_theme->mStandardFontSize;
            mColor = m_theme->mTextColor;
        }
    }

    Vector2i Label::preferredSize(SDL3::SDL_Renderer* ctx) const
    {
        if (m_caption == "")
            return Vector2i::Zero();

        if (m_fixed_size.x > 0)
        {
            int w, h;
            const_cast<Label*>(this)->m_theme->getUtf8Bounds(mFont.c_str(), fontSize(),
                                                             m_caption.c_str(), &w, &h);
            return Vector2i(m_fixed_size.x, h);
        }
        else
        {
            int w, h;
            const_cast<Label*>(this)->m_theme->getUtf8Bounds(mFont.c_str(), fontSize(),
                                                             m_caption.c_str(), &w, &h);
            return Vector2i(w, m_theme->mStandardFontSize);
        }
    }

    void Label::setFontSize(int fontSize)
    {
        Widget::setFontSize(fontSize);
        _texture.dirty = true;
    }

    void Label::draw(SDL3::SDL_Renderer* renderer)
    {
        Widget::draw(renderer);

        if (_texture.dirty)
            m_theme->getTexAndRectUtf8(renderer, _texture, 0, 0, m_caption.c_str(), mFont.c_str(),
                                       fontSize(), mColor);

        if (m_fixed_size.x > 0)
        {
            auto&& pos = absolute_position();
            SDL3::SDL_FRect rect{
                static_cast<float>(pos.x),
                static_cast<float>(pos.y),
                0.0f,
                0.0f,
            };
            SDL3::SDL_RenderTexture(renderer, _texture.tex, &rect, nullptr);
        }
        else
        {
            auto&& pos = absolute_position() +
                         Vector2i(0, int((m_size.y - _texture.rrect.h) * 0.5f));
            SDL3::SDL_FRect rect{
                static_cast<float>(pos.x),
                static_cast<float>(pos.y),
                0.0f,
                0.0f,
            };
            SDL3::SDL_RenderTexture(renderer, _texture.tex, &rect, nullptr);
        }
    }
}
