#pragma once

#include "ui/layouts/layout.hpp"

namespace rl::ui {
    class Widget;

    // Special layout for widgets grouped by labels.
    // This widget resembles a box layout in that it arranges a set of widgets
    // vertically. All widgets are indented on the horizontal axis except for
    // Label widgets, which are not indented. This creates a pleasing layout where a number of
    // widgets are grouped under some high-level heading.
    class GroupLayout final : public OldLayout
    {
    public:
        explicit GroupLayout(const f32 margin = 15.0f, const f32 spacing = 6.0f,
                             const f32 group_spacing = 14.0f, const f32 group_indent = 20.0f)
            : m_margin{ margin }
            , m_spacing{ spacing }
            , m_group_spacing{ group_spacing }
            , m_group_indent{ group_indent }
        {
        }

        f32 margin() const;
        f32 spacing() const;
        f32 group_indent() const;
        f32 group_spacing() const;

        void set_margin(f32 margin);
        void set_spacing(f32 spacing);
        void set_group_indent(f32 group_indent);
        void set_group_spacing(f32 group_spacing);

    public:
        virtual void apply_layout(nvg::Context* nvg_context, const Widget* widget) const override;
        virtual ds::dims<f32> computed_size(nvg::Context* nvg_context,
                                            const Widget* widget) const override;

    protected:
        f32 m_margin{ 15.0f };
        f32 m_spacing{ 0.0f };
        f32 m_group_spacing{ 0.0f };
        f32 m_group_indent{ 0.0f };
    };
}
