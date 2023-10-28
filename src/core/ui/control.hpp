#pragma once

#include <concepts>
#include <memory>
#include <string>
#include <vector>
#include <type_traits>

#include "core/ds/dimensions.hpp"
#include "core/ds/point.hpp"
#include "core/ds/rect.hpp"
#include "core/ds/vector2d.hpp"
#include "core/ui/anchor.hpp"

namespace rl::ui
{
    template <typename T>
    class Widget;

    /**
     * @brief Base class of all widgets/containers
     * */
    class Control
    {
    public:
        inline bool handle_inputs(const input::Input&)
        {
            return false;
        }

        inline bool handle_mouse_inputs(const input::Input&)
        {
            return false;
        }

    protected:
        inline bool move(ds::vector2<int32_t>&& cursor_movement_delta)
        {
            // static_cast<this->element_type>(this)->handle_inputs(input);
            //
            //  TODO: handle collisions..
            //  ds::rect<int32_t> moved{ m_rect + offset };
            //  if (moved.contained_by())
            //      ...

            m_rect += cursor_movement_delta;
            return true;
        }

        inline constexpr bool check_collision(ds::point<int32_t>&& pt)
        {
            return m_rect.overlaps(pt);
        }

        inline constexpr void draw()
        {
            return;
        }

        template <typename TControl>
        constexpr void add_child(TControl&& control)
        {
            m_children.emplace_back(std::move(control));
        }

    protected:
        static inline std::atomic<uint32_t> m_control_count{ 0 };

        bool m_visible{ true };
        bool m_enabled{ true };
        // label text
        std::string m_text{};
        // reference point in local space
        ds::point<int32_t> m_anchor{ 0, 0 };
        // control's size and position, relative to anchor
        ds::rect<int32_t> m_rect{
            { 0, 0 },
            { 0, 0 },
        };

        // all controls/widgets contained by this one
        std::vector<std::type_identity_t<Widget<Control>>*> m_children{};
        const uint32_t m_id{ m_control_count.fetch_add(1, std::memory_order_relaxed) };
    };
}
