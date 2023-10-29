#pragma once

#include <string>
#include <vector>
#include <initializer_list>

#include "core/ds/dimensions.hpp"
#include "core/ds/point.hpp"
#include "core/ds/rect.hpp"
#include "core/ds/vector2d.hpp"
#include "core/input/input.hpp"
#include "ui/properties.hpp"

#include "thirdparty/raygui.hpp"

// #include "thirdparty/raylib.hpp"

namespace rl::ui
{

    enum class InputCaptureError {
        Unknown,
    };

    enum MouseEventCapture {
        Unknown     = 0 << 1,
        None        = 1 << 0,
        Collision   = 1 << 1,
        Grabbed     = 1 << 2,
        Dragging    = 1 << 3,
        PartialDrag = 1 << 4,
        Released    = 1 << 5,
    };

    /**
     * @brief Base class for all UI Elements. This class leverage static polymorphism / upside-down
     * inheritance since it receives the derived control's type as a template arg. Because of this
     * it's able to use a static cast to the derived object to delegate implementation details to
     * the member function overrides defined in the derived classes without the need for v-tables
     * and/or dynamic dispatch at runtime.
     * */
    class control
    {
    public:
        explicit control(properties&& props)
        {
            pos  = std::move(props.position);
            size = std::move(props.size);
            text = std::move(props.text);
        }

        // bool inputs_impl(input::Input& inputs);

        // constexpr inline bool update_gui(this auto&& self, input::Input& inputs);
        // constexpr inline bool draw(this auto&& self);

        constexpr inline decltype(auto) update_gui(this auto&& self, input::Input& inputs)
        {
            return self.inputs_impl(inputs);
        }

        constexpr inline decltype(auto) draw(this auto&& self)
        {
            return self.draw_impl();
        }

        constexpr bool draw_impl()
        {
            return false;
        }

        // constexpr bool inputs_impl(input::Input&)
        // {
        //     return false;
        // }

        constexpr bool inputs_impl(this auto&& self, input::Input& inputs)
        {
            bool inputs_captured{ false };
            for (auto&& child : children)
            {
                if (!child.visible || !child.enabled)
                    continue;

                inputs_captured |= child.update_gui(inputs);
                if (inputs_captured)
                    break;
            }

            if (!inputs_captured)  // && !processed_inputs)
                inputs_captured |= self.inputs_impl(inputs);

            return inputs_captured;
        }

    private:
        control() = default;

    public:
        u64 m_global_count{ 1 };
        u64 id{ m_global_count++ };

        bool visible{ true };
        bool enabled{ true };
        std::string title{};
        std::string text{};
        ds::point<i32> pos{ 0, 0 };
        ds::dimensions<i32> size{ 0, 0 };
        std::vector<control> children{};
    };
}
