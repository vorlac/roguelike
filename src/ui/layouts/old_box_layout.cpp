#include <numeric>
#include <utility>

#include "ds/dims.hpp"
#include "gfx/vg/nanovg.hpp"
#include "ui/layouts/old_box_layout.hpp"
#include "ui/theme.hpp"
#include "ui/widget.hpp"
#include "ui/widgets/dialog.hpp"
#include "ui/widgets/label.hpp"
#include "utils/io.hpp"
#include "utils/logging.hpp"
#include "utils/math.hpp"
#include "utils/numeric.hpp"

namespace rl::ui {
    OldBoxLayout::OldBoxLayout(const Alignment orientation, const Placement_OldAlignment alignment,
                               const f32 margin, const f32 spacing)
        : m_margin{ margin }
        , m_spacing{ spacing }
        , m_orientation{ orientation }
        , m_alignment{ alignment } {
    }

    ds::dims<f32> OldBoxLayout::computed_size(nvg::Context*, const Widget* widget) const {
        ds::dims size{
            2.0f * m_margin,
            2.0f * m_margin,
        };

        f32 y_offset{ 0.0f };
        const auto dialog{ dynamic_cast<const Dialog*>(widget) };
        if (dialog != nullptr && !dialog->title().empty()) {
            const auto header_size{ dialog->header_height() };
            if (m_orientation == Alignment::Vertical)
                size.height += header_size - (m_margin / 2.0f);
            else
                y_offset = header_size;
        }

        bool first_child{ true };
        for (const auto child : widget->children()) {
            if (!child->visible())
                continue;

            if (!first_child)
                size.height += m_spacing;

            const ds::dims ps{ child->preferred_size() };
            const ds::dims fs{ child->fixed_size() };
            const ds::dims target_size{
                math::equal(fs.width, 0.0f) ? ps.width : fs.width,
                math::equal(fs.height, 0.0f) ? ps.height : fs.height,
            };

            first_child = false;
            size.width += target_size.width;
            size.height = std::max(size.height, target_size.height + (m_margin * 2.0f));
        }

        return size + ds::dims{ 0.0f, y_offset };
    }

    void OldBoxLayout::apply_layout(nvg::Context*, const Widget* widget) const {
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
        if (dialog != nullptr && !dialog->title().empty()) {
            if (m_orientation == Alignment::Vertical)
                position += dialog->header_height() - (m_margin / 2.0f);
            else {
                y_offset = dialog->header_height();
                container_size.height -= y_offset;
            }
        }

        bool first_child{ true };
        for (const auto child : widget->children()) {
            if (!child->visible())
                continue;

            if (!first_child)
                position += m_spacing;

            first_child = false;
            const ds::dims ps{ child->preferred_size() };
            const ds::dims fs{ child->fixed_size() };
            ds::dims target_size{
                math::equal(fs.width, 0.0f) ? ps.width : fs.width,
                math::equal(fs.height, 0.0f) ? ps.height : fs.height,
            };

            ds::point<f32> pos{
                0.0f,
                y_offset,
            };

            pos.x = position;
            switch (m_alignment) {
                case Placement_OldAlignment::Minimum:
                    pos.y += m_margin;
                    break;
                case Placement_OldAlignment::Center:
                    pos.y += (container_size.height - target_size.height) / 2.0f;
                    break;
                case Placement_OldAlignment::Maximum:
                    pos.y += container_size.height - target_size.height - m_margin * 2.0f;
                    break;
                case Placement_OldAlignment::Fill:
                    pos.y += m_margin;
                    target_size.height = std::fabs(fs.height) > std::numeric_limits<f32>::epsilon()
                                           ? fs.height
                                           : (container_size.height - m_margin * 2.0f);
                    break;
                case Placement_OldAlignment::None:
                    debug_assert(false);
                    break;
            }

            position += target_size.width;

            child->set_position(pos);
            child->set_size(target_size);
            child->perform_layout();
        }
    }

    Alignment OldBoxLayout::orientation() const {
        return m_orientation;
    }

    void OldBoxLayout::set_orientation(const Alignment orientation) {
        m_orientation = orientation;
    }

    Placement_OldAlignment OldBoxLayout::alignment() const {
        return m_alignment;
    }

    void OldBoxLayout::set_alignment(const Placement_OldAlignment alignment) {
        m_alignment = alignment;
    }

    f32 OldBoxLayout::margin() const {
        return m_margin;
    }

    void OldBoxLayout::set_margin(const f32 margin) {
        m_margin = margin;
    }

    f32 OldBoxLayout::spacing() const {
        return m_spacing;
    }

    void OldBoxLayout::set_spacing(const f32 spacing) {
        m_spacing = spacing;
    }
}
