#pragma once
#include <ranges>
#include <stack>

#include "core/ui/widget.hpp"
#include "graphics/vg/nanovg.hpp"

namespace rl {
    struct LocalTransform
    {
        LocalTransform(const ui::Widget* widget) noexcept
        {
#ifndef NDEBUG
            // TODO: remove this after more testing
            const bool already_in_local_space{
                std::ranges::find_if(m_stack,
                                     [&](const ui::Widget* w) {
                                         return w == widget;
                                     }) != m_stack.end(),
            };

            runtime_assert(already_in_local_space != (m_stack.empty() || m_stack.back() != widget),
                           "????");
#endif
            runtime_assert(widget != nullptr, "invalid reference to UI element");
            if (m_stack.empty() || m_stack.back() != widget)
            {
                nvg::translate(widget->context(), widget->position());
                m_stack.push_back(widget);
            }
        }

        ~LocalTransform() noexcept
        {
            const ui::Widget* widget{ m_stack.back() };
            nvg::translate(widget->context(), -widget->position());
            m_stack.pop_back();
        }

        static inline std::vector<const ui::Widget*> m_stack = {};
    };
}
