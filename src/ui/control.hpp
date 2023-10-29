#pragma once

#include <string>
#include <vector>
#include <initializer_list>

#include "core/ds/dimensions.hpp"
#include "core/ds/point.hpp"
#include "core/ds/rect.hpp"
#include "core/ds/vector2d.hpp"
#include "core/input/input.hpp"
#include "thirdparty/raygui.hpp"
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
        explicit control(properties&& props)
        {
            this->pos  = std::move(props.position);
            this->size = std::move(props.size);
            this->text = std::move(props.text);
        }

        bool update(this auto&& self, input::Input& inputs)
        {
            return self.inputs_impl(inputs);
        }

        inline bool draw(this auto&& self)
        {
            return self.draw_impl();
        }

        bool inputs_impl(input::Input& inputs)
        {
            bool inputs_captured{ false };
            for (auto&& child : children)
            {
                if (!child.visible || !child.enabled)
                    continue;

                inputs_captured |= child.update(inputs);
                if (inputs_captured)
                    break;
            }

            if (!inputs_captured)  // && !processed_inputs)
                inputs_captured |= this->inputs_impl(inputs);

            return inputs_captured;
        }

        bool draw_impl()
        {
            return false;
        }

    private:
        control() = delete;

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
