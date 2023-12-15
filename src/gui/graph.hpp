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

        void setCaption(const std::string& caption)
        {
            m_caption = caption;
            _captionTex.dirty = true;
        }

        const std::string& header() const
        {
            return mHeader;
        }

        void setHeader(const std::string& header)
        {
            mHeader = header;
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

        const Color& backgroundColor() const
        {
            return mBackgroundColor;
        }

        void setBackgroundColor(const Color& backgroundColor)
        {
            mBackgroundColor = backgroundColor;
        }

        const Color& foregroundColor() const
        {
            return mForegroundColor;
        }

        void setForegroundColor(const Color& foregroundColor)
        {
            mForegroundColor = foregroundColor;
        }

        const Color& textColor() const
        {
            return mTextColor;
        }

        void setTextColor(const Color& textColor)
        {
            mTextColor = textColor;
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

        Vector2i preferredSize(SDL3::SDL_Renderer* ctx) const override;
        void draw(SDL3::SDL_Renderer* ctx) override;

    protected:
        std::string m_caption, mHeader, mFooter;
        Color mBackgroundColor, mForegroundColor, mTextColor;
        std::vector<float> mValues;

        Texture _captionTex;
        Texture _headerTex;
        Texture _footerTex;

        struct AsyncTexture;
        typedef std::shared_ptr<AsyncTexture> AsyncTexturePtr;
        AsyncTexturePtr _atx;
    };
}
