#pragma once

#include "scroll_dialog.hpp"
#include "utils/numeric.hpp"

namespace rl::ui {

    class Popup final : public ScrollableDialog {
    public:
        explicit Popup(Widget* parent, ScrollableDialog* parent_dialog = nullptr);

        f32 anchor_offset() const;
        f32 anchor_size() const;
        Side side() const;
        ScrollableDialog* parent_dialog();
        const ScrollableDialog* parent_dialog() const;
        ds::point<f32> anchor_pos() const;

        void set_anchor_pos(ds::point<f32> anchor_pos);
        void set_anchor_offset(f32 anchor_offset);
        void set_anchor_size(f32 anchor_size);
        void set_side(Side popup_side);

    public:
        virtual void draw() override;
        virtual void perform_layout() override;
        virtual void refresh_relative_placement() override;

    protected:
        ScrollableDialog* m_parent_dialog{ nullptr };
        ds::point<f32> m_anchor_pos{ 0.0f, 0.0f };
        f32 m_anchor_offset{ 30.0f };
        f32 m_anchor_size{ 15.0f };
        Side m_side{ Side::Right };
    };
}
