#pragma once

#include "ds/dims.hpp"
#include "ui/widget.hpp"

namespace rl::ui {

    class ProgressBar final : public Widget {
    public:
        explicit ProgressBar(Widget* parent);

        f32 value() const;
        void set_value(f32 value);

    public:
        virtual ds::dims<f32> preferred_size() const override;
        virtual void draw() override;

    protected:
        f32 m_value{ 0.0f };
    };
}
