#pragma once

#include <memory>
#include <string>
#include <vector>

#include "core/ds/dimensions.hpp"
#include "core/ds/point.hpp"
#include "core/ds/rect.hpp"
#include "core/ds/vector2d.hpp"
#include "core/ui/anchor.hpp"

namespace rl::ui
{
    /**
     * @brief Base class of all widgets/containers
     * */
    class Control
    {
    public:
        enum class Type {
            Control,
            Dialog,
            Button,
            ToggleButton,
            Checkbox,
        };

        struct Margins
        {
            uint32_t top{ 0 };
            uint32_t bottom{ 0 };
            uint32_t left{ 0 };
            uint32_t right{ 0 };
        };

    public:
        constexpr Control() = default;

        constexpr Control(Control* other)
        {
            if (other == nullptr) [[unlikely]]
                return;
            *this = std::move(*other);
        }

        Control(const Control& other)
        {
            *this = other;
        }

        Control(Control&& other)
        {
            *this = std::move(other);
        }

        constexpr Control(int32_t x, int32_t y, int32_t width, int32_t height, std::string text = "")
            : m_text{ text }
            , m_anchor{ x, y }
            , m_rect{ x, y, width, height }
        {
        }

        constexpr Control(ds::point<int32_t> anchor, ds::rect<int32_t> rect, std::string text = "")
            : m_text{ text }
            , m_anchor{ anchor }
            , m_rect{ rect }
        {
        }

        constexpr Control(ds::point<int32_t> anchor, ds::point<int32_t>, ds::rect<int32_t> rect,
                          std::string text)
            : m_text{ text }
            , m_anchor{ anchor }
            , m_rect{ rect }
        {
        }

        constexpr virtual ~Control() = default;

        Control& operator=(const Control& other)
        {
            m_anchor   = other.m_anchor;
            m_children = other.m_children;
            m_rect     = other.m_rect;
            m_text     = other.m_text;
            m_visible  = other.m_visible;
            m_type     = other.m_type;
            return *this;
        }

        Control& operator=(Control&& other)
        {
            m_anchor   = std::move(other.m_anchor);
            m_children = std::move(other.m_children);
            m_rect     = std::move(other.m_rect);
            m_text     = std::move(other.m_text);
            m_visible  = other.m_visible;
            m_type     = other.m_type;
            return *this;
        }

        /**
         * @brief Moves a control by some delta.
         * @returns false if it's unable to move
         * the full distance due to a collision
         * */
        constexpr bool move(const ds::vector2<int32_t>& offset)
        {
            // TODO: handle collisions..
            // ds::rect<int32_t> moved{ m_rect + offset };
            // if (moved.contained_by())
            //     ...

            m_rect += offset;
            return true;
        }

        constexpr virtual bool check_collision(ds::point<int32_t>)
        {
            return false;
        }

        virtual void draw()
        {
            return;
        }

        constexpr void visible(bool visible)
        {
            m_visible = visible;
        }

        constexpr bool visible() const
        {
            return m_visible;
        }

        virtual void setup()
        {
            return;
        }

        virtual void teardown()
        {
            return;
        }

        constexpr void add_child(Control* control)
        {
            m_children.emplace_back(control);
        }

        inline void set_type(Control::Type type)
        {
            m_type = type;
        }

        inline Control::Type type() const
        {
            return m_type;
        }

    protected:
        bool m_visible{ true };
        // label text
        std::string m_text{};
        // reference point in local space
        ds::point<int32_t> m_anchor{ 0, 0 };
        // control's size and position, relative to anchor
        ds::rect<int32_t> m_rect{
            ds::point<int32_t>{ 0, 0 },
            ds::dimensions<int32_t>{ 0, 0 },
        };
        // all controls/widgets contained by this one
        std::vector<std::shared_ptr<Control>> m_children{};
        Control::Type m_type{ Control::Type::Control };
    };
}
