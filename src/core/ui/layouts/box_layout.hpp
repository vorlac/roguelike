#pragma once

#include "core/ui/layouts/layout.hpp"
#include "ds/color.hpp"
#include "ds/dims.hpp"
#include "graphics/vg/nanovg.hpp"
#include "utils/numeric.hpp"

namespace rl::ui {
    class Widget;

    // Simple layout that supports horizontal and vertical orientation.
    // Aside form defining the Layout interface for sizing and
    // performing the Layout, a BoxLayout only handles basic orientation,
    // margins and spacing.
    class BoxLayout final : public Layout
    {
    public:
        explicit BoxLayout(Orientation orientation,                  // horizontal or vertical
                           Alignment alignment = Alignment::Center,  // min, middle, max, full
                           f32 margin = 0.0f,                        // the widget margins
                           f32 spacing = 0.0f);                      // the widget spacing

        f32 margin() const;
        f32 spacing() const;
        Alignment alignment() const;
        Orientation orientation() const;

        void set_margin(f32 margin);
        void set_spacing(f32 spacing);
        void set_orientation(Orientation orientation);
        void set_alignment(Alignment alignment);

    public:
        virtual void perform_layout(nvg::Context* nvg_context, Widget* widget) const override;
        virtual ds::dims<f32> preferred_size(nvg::Context* nvg_context,
                                             const Widget* widget) const override;

    protected:
        f32 m_margin{ 0.0f };
        f32 m_spacing{ 0.0f };
        Orientation m_orientation{};
        Alignment m_alignment{};
    };
}
