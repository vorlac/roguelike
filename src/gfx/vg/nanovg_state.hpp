#pragma once
#include <ranges>

#include "gfx/vg/nanovg.hpp"
#include "ui/widget.hpp"

namespace rl {
    struct LocalTransform final
    {
    public:
        LocalTransform() = delete;
        LocalTransform(const LocalTransform&) = delete;
        LocalTransform(LocalTransform&&) = delete;
        LocalTransform& operator=(LocalTransform&&) = delete;
        LocalTransform& operator=(const LocalTransform&) = delete;

    public:
        explicit LocalTransform(const ui::Widget* widget) noexcept
        {
            debug_assert(widget != nullptr, "invalid reference to UI element");
            if (scope_stack.empty() || scope_stack.back() != widget) {
                absolute_pos += widget->position();
                nvg::translate(ui::Widget::context(), widget->position());
                scope_stack.push_back(widget);
                m_added_to_stack = true;
            }
        }

        ~LocalTransform()
        {
            if (m_added_to_stack) {
                const ui::Widget* widget{ scope_stack.back() };
                absolute_pos -= widget->position();
                nvg::translate(ui::Widget::context(), -widget->position());
                scope_stack.pop_back();
            }
        }

    public:
        static inline std::vector<const ui::Widget*> scope_stack{};
        static inline ds::point<f32> absolute_pos{ 0.0f, 0.0f };

    private:
        bool m_added_to_stack{ false };
    };

}
