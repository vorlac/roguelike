#pragma once

#include "gui/widget.hpp"

namespace rl::gui {
    /**
     * \class Label label.h sdl_gui/label.h
     *
     * \brief Text label widget.
     *
     * The font and color can be customized. When \ref Widget::set_fixed_width()
     * is used, the text is wrapped when it surpasses the specified width.
     */
    class Label : public Widget
    {
    public:
        Label(Widget* parent, const std::string& caption, const std::string& font = "sans",
              int fontSize = -1);

        /// Get the label's text caption
        const std::string& caption() const
        {
            return m_caption;
        }

        /// Set the label's text caption
        void setCaption(const std::string& caption)
        {
            m_caption = caption;
            _texture.dirty = true;
        }

        /// Set the currently active font (2 are available by default: 'sans' and 'sans-bold')
        void setFont(const std::string& font)
        {
            mFont = font;
        }

        /// Get the currently active font
        const std::string& font() const
        {
            return mFont;
        }

        /// Get the label color
        Color color() const
        {
            return mColor;
        }

        /// Set the label color
        void setColor(const Color& color)
        {
            mColor = color;
        }

        /// Set the \ref Theme used to draw this widget
        virtual void set_theme(Theme* theme) override;

        /// Compute the size needed to fully display the label
        virtual Vector2i preferredSize(SDL3::SDL_Renderer* ctx) const override;

        /// Draw the label
        void draw(SDL3::SDL_Renderer* renderer) override;
        void setFontSize(int fontSize) override;

    protected:
        std::string m_caption;
        std::string mFont;
        Color mColor;
        Texture _texture;
    };
}
