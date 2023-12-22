#pragma once

#include "gui/widget.hpp"

namespace rl::gui {

    /**
     * \class ProgressBar progressbar.h nanogui/progressbar.h
     *
     * \brief Standard widget for visualizing progress.
     */
    class ProgressBar : public Widget
    {
    public:
        ProgressBar(Widget* parent);

        float value()
        {
            return m_value;
        }

        void set_value(float value)
        {
            m_value = value;
        }

        virtual Vector2i preferred_size(NVGcontext* ctx) const override;
        virtual void draw(NVGcontext* ctx) override;

    protected:
        float m_value;
    };

}
