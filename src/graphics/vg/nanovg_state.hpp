#pragma once
#include <stack>

#include "core/ui/widget.hpp"
#include "graphics/vg/nanovg.hpp"

namespace rl {
    struct LocalTransform
    {
        inline LocalTransform(ui::Widget* widget) noexcept
        {
            runtime_assert(widget != nullptr, "invalid reference to UI element");
            if (m_stack.empty() || m_stack.top() != widget)
            {
                nvg::Translate(widget->context(), widget->position());
                m_stack.push(widget);
            }
        }

        inline ~LocalTransform() noexcept
        {
            ui::Widget* widget{ m_stack.top() };
            nvg::Translate(widget->context(), -widget->position());
            m_stack.pop();
        }

        static inline std::stack<ui::Widget*> m_stack = {};
    };

}
