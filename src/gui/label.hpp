#pragma once

#include "gui/widget.hpp"

namespace rl::gui {

    /**
     * \class Label label.h nanogui/label.h
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
              int font_size = -1);

        /// Get the label's text caption
        const std::string& caption() const
        {
            return m_caption;
        }

        /// Set the label's text caption
        void set_caption(const std::string& caption)
        {
            m_caption = caption;
        }

        /// Set the currently active font (2 are available by default: 'sans' and 'sans-bold')
        void set_font(const std::string& font)
        {
            m_font = font;
        }

        /// Get the currently active font
        const std::string& font() const
        {
            return m_font;
        }

        /// Get the label color
        Color color() const
        {
            return m_color;
        }

        /// Set the label color
        void set_color(const Color& color)
        {
            m_color = color;
        }

        /// Set the \ref Theme used to draw this widget
        virtual void set_theme(Theme* theme) override;

        /// Compute the size needed to fully display the label
        virtual Vector2i preferred_size(NVGcontext* ctx) const override;

        /// Draw the label
        virtual void draw(NVGcontext* ctx) override;

    protected:
        std::string m_caption;
        std::string m_font;
        Color m_color;
    };
}
