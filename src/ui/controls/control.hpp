#pragma once

#include <atomic>
#include <memory>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

#include "core/ds/dimensions.hpp"
#include "core/ds/point.hpp"
#include "core/ds/vector2d.hpp"
#include "core/input/input.hpp"
#include "core/numerics.hpp"
#include "core/utils/io.hpp"
#include "ui/controls/control.hpp"
#include "ui/properties.hpp"

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
    public:
        inline bool update(this auto&& self, input::Input& inputs)
        {
            bool inputs_captured{ false };

            for (auto&& child : self.children)
            {
                if (!child.visible || !child.enabled)
                    continue;

                inputs_captured |= child.update(inputs);
                if (inputs_captured)
                    break;
            }

            if (!inputs_captured)  // && !processed_inputs)
                inputs_captured |= self.inputs_impl(inputs);

            return inputs_captured;
        }

        inline bool draw(this auto&& self)
        {
            return self.draw_impl();
        }

        inline bool inputs_impl(input::Input&)
        {
            log::info("{}::handle_inputs_impl()", name());
            return false;
        }

        inline bool draw_impl()
        {
            log::info("{}::draw_controls_impl()", this->name());
            return false;
        }

        constexpr control(ui::properties&& props)
        {
            this->pos  = std::move(props.position);
            this->size = std::move(props.size);
            this->text = std::move(props.text);
        }

        static inline std::atomic<u64> m_global_count{ 1 };

    private:
        constexpr control() = delete;

        constexpr inline std::string name()
        {
            return "ControlBase";
        }

    public:
        mutable u64 id{ 0 };

        bool visible{ true };
        bool enabled{ true };
        std::string title{};
        std::string text{};
        ds::point<i32> pos{ 0, 0 };
        ds::dimensions<i32> size{ 0, 0 };
        std::vector<ui::control> children{};
    };
}
