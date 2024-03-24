#include <numeric>
#include <utility>

#include "core/ui/layouts/box_layout.hpp"
#include "core/ui/theme.hpp"
#include "core/ui/widget.hpp"
#include "core/ui/widgets/dialog.hpp"
#include "core/ui/widgets/label.hpp"
#include "ds/dims.hpp"
#include "graphics/vg/nanovg.hpp"
#include "utils/io.hpp"
#include "utils/logging.hpp"
#include "utils/math.hpp"
#include "utils/numeric.hpp"

namespace rl::ui {
    BoxLayout::BoxLayout(const Orientation orientation, const Alignment alignment, const f32 margin,
                         const f32 spacing)
        : m_margin{ margin }
        , m_spacing{ spacing }
        , m_orientation{ orientation }
        , m_alignment{ alignment }
    {
    }

    ds::dims<f32> BoxLayout::preferred_size(nvg::Context* nvg_context, const Widget* widget) const
    {
        ds::dims size{
            2.0f * m_margin,
            2.0f * m_margin,
        };

        f32 y_offset{ 0.0f };
        const auto dialog{ dynamic_cast<const Dialog*>(widget) };
        if (dialog != nullptr && !dialog->title().empty())
        {
            const auto header_size{ dialog->header_height() };
            if (m_orientation == Orientation::Vertical)
                size.height += header_size - (m_margin / 2.0f);
            else
                y_offset = header_size;
        }

        bool first_child{ true };
        for (const auto child : widget->children())
        {
            if (!child->visible())
                continue;

            if (!first_child)
                size.height += m_spacing;

            const ds::dims ps{ child->preferred_size() };
            const ds::dims fs{ child->fixed_size() };
            const ds::dims target_size{
                fs.width == 0.0f ? ps.width : fs.width,
                fs.height == 0.0f ? ps.height : fs.height,
            };

            first_child = false;
            size.width += target_size.width;
            size.height = std::max(size.height, target_size.height + (m_margin * 2.0f));
        }

        return size + ds::dims{ 0.0f, y_offset };
    }

    void BoxLayout::perform_layout(nvg::Context* nvg_context, const Widget* widget) const
    {
        const ds::dims fs_w{ widget->fixed_size() };
        ds::dims container_size{
            std::fabs(fs_w.width) > std::numeric_limits<f32>::epsilon() ? fs_w.width
                                                                        : widget->width(),
            std::fabs(fs_w.height) > std::numeric_limits<f32>::epsilon() ? fs_w.height
                                                                         : widget->height(),
        };

        f32 position{ m_margin };
        f32 y_offset{ 0.0f };

        const auto dialog{ dynamic_cast<const Dialog*>(widget) };
        if (dialog != nullptr && !dialog->title().empty())
        {
            if (m_orientation == Orientation::Vertical)
                position += dialog->header_height() - (m_margin / 2.0f);
            else
            {
                y_offset = dialog->header_height();
                container_size.height -= y_offset;
            }
        }

        bool first_child{ true };
        for (const auto child : widget->children())
        {
            if (!child->visible())
                continue;

            if (!first_child)
                position += m_spacing;

            first_child = false;
            const ds::dims ps{ child->preferred_size() };
            const ds::dims fs{ child->fixed_size() };
            ds::dims target_size{
                std::fabs(fs.width) > std::numeric_limits<f32>::epsilon() ? fs.width : ps.width,
                std::fabs(fs.height) > std::numeric_limits<f32>::epsilon() ? fs.height : ps.height,
            };

            ds::point pos{
                0.0f,
                y_offset,
            };

            pos.x = position;
            switch (m_alignment)
            {
                case Alignment::Minimum:
                    pos.y += m_margin;
                    break;
                case Alignment::Center:
                    pos.y += (container_size.height - target_size.height) / 2.0f;
                    break;
                case Alignment::Maximum:
                    pos.y += container_size.height - target_size.height - m_margin * 2.0f;
                    break;
                case Alignment::Fill:
                    pos.y += m_margin;
                    target_size.height = std::fabs(fs.height) > std::numeric_limits<f32>::epsilon()
                                           ? fs.height
                                           : (container_size.height - m_margin * 2.0f);
                    break;
                case Alignment::None:
                    assert_cond(false);
                    break;
            }

            position += target_size.width;

            child->set_position(std::move(pos));
            child->set_size(std::move(target_size));
            child->perform_layout();
        }
    }

    Orientation BoxLayout::orientation() const
    {
        return m_orientation;
    }

    void BoxLayout::set_orientation(const Orientation orientation)
    {
        m_orientation = orientation;
    }

    Alignment BoxLayout::alignment() const
    {
        return m_alignment;
    }

    void BoxLayout::set_alignment(const Alignment alignment)
    {
        m_alignment = alignment;
    }

    f32 BoxLayout::margin() const
    {
        return m_margin;
    }

    void BoxLayout::set_margin(const f32 margin)
    {
        m_margin = margin;
    }

    f32 BoxLayout::spacing() const
    {
        return m_spacing;
    }

    void BoxLayout::set_spacing(const f32 spacing)
    {
        m_spacing = spacing;
    }
}