#pragma once

#include <array>
#include <cmath>
#include <limits>
#include <numeric>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <vector>

#include "anchor.hpp"
#include "ds/color.hpp"
#include "ds/refcounted.hpp"
#include "ds/shared.hpp"
#include "graphics/vg/nanovg.hpp"
#include "utils/conversions.hpp"
#include "utils/numeric.hpp"
#include "utils/properties.hpp"

namespace rl::ui {
    class Widget;

    class Layout : public ds::refcounted
    {
    public:
        template <typename T>
        struct Spacing
        {
            T horizontal{};
            T vertical{};
        };

    public:
        // Performs applies all Layout computations for the given widget.
        virtual void perform_layout(nvg::Context* nvc, const Widget* w) const = 0;
        // Compute the preferred size for a given Layout and widget
        virtual ds::dims<f32> preferred_size(nvg::Context* nvc, const Widget* w) const = 0;

        std::string name() const
        {
            return typeid(*this).name();
        }
    };
}
