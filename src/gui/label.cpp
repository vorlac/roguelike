#include "gui/label.hpp"
#include "gui/theme.hpp"

namespace rl::gui {
    Label::Label(Widget* parent, const std::string& caption, const std::string& font, int font_size)
        : Widget(parent)
        , m_caption(caption)
        , m_font(font)
    {
        if (m_theme)
        {
            m_font_size = m_theme->m_standard_font_size;
            m_color = m_theme->m_text_color;
        }

        if (font_size >= 0)
            m_font_size = font_size;

        m_texture.dirty = true;
    }

    void Label::set_theme(Theme* theme)
    {
        Widget::set_theme(theme);
        if (m_theme)
        {
            m_font_size = m_theme->m_standard_font_size;
            m_color = m_theme->m_text_color;
        }
    }

    Vector2i Label::preferred_size(SDL3::SDL_Renderer* ctx) const
    {
        if (m_caption == "")
            return Vector2i::zero();

        if (m_fixed_size.x > 0)
        {
            int w, h;
            const_cast<Label*>(this)->m_theme->get_utf8_bounds(m_font.c_str(), font_size(),
                                                               m_caption.c_str(), &w, &h);
            return Vector2i(m_fixed_size.x, h);
        }
        else
        {
            int w, h;
            const_cast<Label*>(this)->m_theme->get_utf8_bounds(m_font.c_str(), font_size(),
                                                               m_caption.c_str(), &w, &h);
            return Vector2i(w, m_theme->m_standard_font_size);
        }
    }

    void Label::set_font_size(int font_size)
    {
        Widget::set_font_size(font_size);
        m_texture.dirty = true;
    }

    void Label::draw(SDL3::SDL_Renderer* renderer)
    {
        Widget::draw(renderer);

        if (m_texture.dirty)
            m_theme->get_texture_and_rect_utf8(renderer, m_texture, 0, 0, m_caption.c_str(),
                                               m_font.c_str(), font_size(), m_color);

        if (m_fixed_size.x > 0)
        {
            auto&& pos = absolute_position();
            SDL3::SDL_FRect rect{
                static_cast<float>(pos.x),
                static_cast<float>(pos.y),
                0.0f,
                0.0f,
            };
            SDL3::SDL_RenderTexture(renderer, m_texture.tex, &rect, nullptr);
        }
        else
        {
            auto&& pos = absolute_position() +
                         Vector2i(0, int((m_size.y - m_texture.rrect.h) * 0.5f));
            SDL3::SDL_FRect rect{
                static_cast<float>(pos.x),
                static_cast<float>(pos.y),
                0.0f,
                0.0f,
            };
            SDL3::SDL_RenderTexture(renderer, m_texture.tex, &rect, nullptr);
        }
    }
}
