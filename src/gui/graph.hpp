#pragma once

#include <memory>

#include "gui/widget.hpp"

namespace rl::gui {
    class Graph : public Widget
    {
    public:
        Graph(Widget* parent, const std::string& caption = "Untitled");

        const std::string& caption() const
        {
            return m_caption;
        }

        void set_caption(const std::string& caption)
        {
            m_caption = caption;
            m_caption_texture.dirty = true;
        }

        const std::string& header() const
        {
            return m_tab_header;
        }

        void setHeader(const std::string& header)
        {
            m_tab_header = header;
            _headerTex.dirty = true;
        }

        const std::string& footer() const
        {
            return mFooter;
        }

        void setFooter(const std::string& footer)
        {
            mFooter = footer;
            _footerTex.dirty = true;
        }

        const Color& background_color() const
        {
            return m_background_color;
        }

        void set_background_color(const Color& background_color)
        {
            m_background_color = background_color;
        }

        const Color& foregroundColor() const
        {
            return mForegroundColor;
        }

        void setForegroundColor(const Color& foregroundColor)
        {
            mForegroundColor = foregroundColor;
        }

        const Color& text_color() const
        {
            return m_text_color;
        }

        void set_text_color(const Color& text_color)
        {
            m_text_color = text_color;
        }

        const std::vector<float>& values() const
        {
            return mValues;
        }

        std::vector<float>& values()
        {
            return mValues;
        }

        void setValues(const std::vector<float>& values)
        {
            mValues = values;
        }

        Vector2i preferred_size(SDL3::SDL_Renderer* ctx) const override;
        void draw(const std::unique_ptr<rl::Renderer>& renderer) override;
        void draw(SDL3::SDL_Renderer* ctx) override;

    protected:
        std::string m_caption, m_tab_header, mFooter;
        Color m_background_color, mForegroundColor, m_text_color;
        std::vector<float> mValues;

        Texture m_caption_texture;
        Texture _headerTex;
        Texture _footerTex;

        struct AsyncTexture;
        typedef std::shared_ptr<AsyncTexture> AsyncTexturePtr;
        AsyncTexturePtr _atx;
    };
}
