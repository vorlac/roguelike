#pragma once

#include <atomic>
#include <expected>
#include <vector>

#include "core/ds/point.hpp"
#include "core/ds/vector2d.hpp"
#include "core/input/input.hpp"
#include "core/ui/control.hpp"

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
        PartialDrag = 1 << 3,
        Released    = 1 << 4,
    };

    /**
     * @brief Base class for all UI Elements. This class leverage static polymorphism / upside-down
     * inheritance since it receives the derived control's type as a template arg. Because of this
     * it's able to use a static cast to the derived object to delegate implementation details to
     * the member function overrides defined in the derived classes without the need for v-tables
     * and/or dynamic dispatch at runtime.
     * */
    template <typename TControl>
    class Widget : public Control
    {
    public:
        using control_type = std::type_identity_t<TControl>;
        using element_type = std::type_identity_t<Widget<TControl>>;

    public:
        inline bool handle_inputs(input::Input& input)
        {
            bool input_event_captured = false;

            // let all children process inputs depth first since
            // they should all be at a higher level than the this
            // current control object (i.e. buttons within a panel)
            for (auto&& child : m_children)
            {
                // skip disabled/invisble controls
                // if (!child->m_visible || !child->m_enabled)
                //   continue;

                // call recursively until
                input_event_captured |= child->handle_inputs(input);
                if (input_event_captured)
                    break;
            }

            if (input_event_captured)
                return input_event_captured;

            auto res{ static_cast<TControl*>(this)->handle_mouse_inputs(input) };
            return res && input_event_captured;
        }

        inline void update()
        {
            static_cast<TControl*>(this)->process_updates();
        }

        inline void render()
        {
            static_cast<TControl*>(this)->draw();
        }

    private:
        /**
         * @brief Default mouse handling implementation
         * */
        inline auto handle_mouse_inputs(input::device::Mouse& mouse)
            -> std::expected<MouseEventCapture, InputCaptureError>
        {
            return MouseEventCapture::None;
            ds::point<int32_t> cursor_movement_delta{ mouse.get_delta() };
        }

        /**
         * @brief Default control updates
         * */
        void process_updates()
        {
            return;
        }

        void draw()
        {
            return;
        }
    };
}
