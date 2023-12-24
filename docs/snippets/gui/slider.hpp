#pragma once

#include "gui/widget.hpp"

namespace rl::gui {

    /**
     * \class Slider slider.h nanogui/slider.h
     *
     * \brief Fractional slider widget with mouse control.
     */
    class Slider : public Widget
    {
    public:
        Slider(Widget* parent);

        float value() const
        {
            return m_value;
        }

        void set_value(float value)
        {
            m_value = value;
        }

        const Color& highlight_color() const
        {
            return m_highlight_color;
        }

        void set_highlight_color(const Color& highlight_color)
        {
            m_highlight_color = highlight_color;
        }

        std::pair<float, float> range() const
        {
            return m_range;
        }

        void set_range(std::pair<float, float> range)
        {
            m_range = range;
        }

        std::pair<float, float> highlighted_range() const
        {
            return m_highlighted_range;
        }

        void set_highlighted_range(std::pair<float, float> highlighted_range)
        {
            m_highlighted_range = highlighted_range;
        }

        const std::function<void(float)>& callback() const
        {
            return m_callback;
        }

        void set_callback(const std::function<void(float)>& callback)
        {
            m_callback = callback;
        }

        const std::function<void(float)>& final_callback() const
        {
            return m_final_callback;
        }

        void set_final_callback(const std::function<void(float)>& callback)
        {
            m_final_callback = callback;
        }

        virtual Vector2i preferred_size(NVGcontext* ctx) const override;
        virtual bool mouse_drag_event(const Vector2i& p, const Vector2i& rel, int button,
                                      int modifiers) override;
        virtual bool mouse_button_event(const Vector2i& p, int button, bool down,
                                        int modifiers) override;
        virtual void draw(NVGcontext* ctx) override;

    protected:
        float m_value;
        std::function<void(float)> m_callback;
        std::function<void(float)> m_final_callback;
        std::pair<float, float> m_range;
        std::pair<float, float> m_highlighted_range;
        Color m_highlight_color;
    };

}
