#pragma once

#include "core/ui/dialog.hpp"
#include "utils/numeric.hpp"

namespace rl::ui {

    class Popup : public ui::Dialog
    {
    public:
        enum class Side {
            Left = 0,
            Right
        };

        Popup(ui::Widget* parent, ui::Dialog* parent_dialog = nullptr);

        f32 anchor_offset() const;
        f32 anchor_size() const;
        Side side() const;
        ui::Dialog* parent_window();
        const ui::Dialog* parent_window() const;
        const ds::point<f32> anchor_pos() const;

        void set_anchor_pos(const ds::point<f32>& anchor_pos);
        void set_anchor_offset(f32 anchor_offset);
        void set_anchor_size(f32 anchor_size);
        void set_side(Side popup_side);

    public:
        virtual void draw(vg::NVGcontext* nvg_context) override;
        virtual void perform_layout(vg::NVGcontext* nvg_context) override;
        virtual void refresh_relative_placement() override;

    protected:
        ui::Dialog* m_parent_dialog{ nullptr };
        ds::point<f32> m_anchor_pos{ 0.0f, 0.0f };
        f32 m_anchor_offset{ 0.0f };
        f32 m_anchor_size{ 0.0f };
        ui::Popup::Side m_side{ Popup::Side::Left };
    };
}
