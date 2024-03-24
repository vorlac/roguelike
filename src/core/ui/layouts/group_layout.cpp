#include <numeric>
#include <utility>

#include "core/ui/layouts/group_layout.hpp"
#include "core/ui/theme.hpp"
#include "core/ui/widget.hpp"
#include "core/ui/widgets/dialog.hpp"
#include "core/ui/widgets/label.hpp"
#include "ds/dims.hpp"
#include "graphics/vg/nanovg.hpp"
#include "utils/io.hpp"
#include "utils/logging.hpp"
#include "utils/numeric.hpp"

namespace rl::ui {
    ds::dims<f32> GroupLayout::preferred_size(nvg::Context* nvg_context, const Widget* widget) const
    {
        scoped_trace(log_level::debug);

        f32 height{ m_margin };
        f32 width{ 2.0f * m_margin };

        const auto dialog{ dynamic_cast<const Dialog*>(widget) };
        if (dialog != nullptr && !dialog->title().empty())
            height += dialog->header_height() - (m_margin / 2.0f);

        bool first{ true };
        bool indent{ false };
        for (const auto c : widget->children())
        {
            if (!c->visible())
                continue;

            const auto label{ dynamic_cast<const Label*>(c) };
            if (!first)
                height += (label == nullptr) ? m_spacing : m_group_spacing;

            first = false;
            ds::dims ps{ c->preferred_size() };
            ds::dims fs{ c->fixed_size() };
            const ds::dims target_size{
                fs.width == 0.0f ? ps.width : fs.width,
                fs.height == 0.0f ? ps.height : fs.height,
            };

            const bool indent_cur{ indent && label == nullptr };
            height += target_size.height;
            width = std::max(width, target_size.width + (2.0f * m_margin) +
                                        (indent_cur ? m_group_indent : 0.0f));
            if (label != nullptr)
                indent = !label->text().empty();
        }

        height += m_margin;

        return ds::dims{ width, height };
    }

    void GroupLayout::perform_layout(nvg::Context* nvg_context, const Widget* widget) const
    {
        scoped_trace(log_level::debug);

        f32 height{ m_margin };
        const f32 available_width{ widget->fixed_width() == 0.0f ? widget->width() - 2.0f * m_margin
                                                                 : widget->fixed_width() };

        const auto dialog{ dynamic_cast<const Dialog*>(widget) };
        if (dialog != nullptr && !dialog->title().empty())
            height += dialog->header_height() - m_margin / 2.0f;

        bool indent{ false };
        bool first_child{ true };
        for (const auto child : widget->children())
        {
            if (!child->visible())
                continue;

            const auto label{ dynamic_cast<const Label*>(child) };
            if (!first_child)
                height += (label == nullptr) ? m_spacing : m_group_spacing;

            first_child = false;
            const bool indent_cur{ indent && label == nullptr };

            const ds::dims fs{ child->fixed_size() };
            const ds::dims ps{
                available_width - (indent_cur ? m_group_indent : 0.0f),
                child->preferred_size().height,
            };

            ds::dims target_size{
                fs.width == 0.0f ? ps.width : fs.width,
                fs.height == 0.0f ? ps.height : fs.height,
            };

            child->set_position({
                m_margin + (indent_cur ? m_group_indent : 0.0f),
                height,
            });

            child->set_size(std::forward<ds::dims<f32>>(target_size));
            child->perform_layout();

            height += target_size.height;
            if (label != nullptr)
                indent = !label->text().empty();
        }
    }

    f32 GroupLayout::margin() const
    {
        scoped_trace(log_level::debug);
        return m_margin;
    }

    void GroupLayout::set_margin(const f32 margin)
    {
        scoped_trace(log_level::debug);
        m_margin = margin;
    }

    f32 GroupLayout::spacing() const
    {
        scoped_trace(log_level::debug);
        return m_spacing;
    }

    void GroupLayout::set_spacing(const f32 spacing)
    {
        scoped_trace(log_level::debug);
        m_spacing = spacing;
    }

    f32 GroupLayout::group_indent() const
    {
        scoped_trace(log_level::debug);
        return m_group_indent;
    }

    void GroupLayout::set_group_indent(const f32 group_indent)
    {
        scoped_trace(log_level::debug);
        m_group_indent = group_indent;
    }

    f32 GroupLayout::group_spacing() const
    {
        scoped_trace(log_level::debug);
        return m_group_spacing;
    }

    void GroupLayout::set_group_spacing(const f32 group_spacing)
    {
        scoped_trace(log_level::debug);
        m_group_spacing = group_spacing;
    }

}