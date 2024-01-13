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

        i32 anchor_offset() const;
        i32 anchor_size() const;
        Side side() const;
        ui::Dialog* parent_window();
        const ui::Dialog* parent_window() const;
        const ds::point<i32> anchor_pos() const;

        void set_anchor_pos(const ds::point<i32>& anchor_pos);
        void set_anchor_offset(i32 anchor_offset);
        void set_anchor_size(i32 anchor_size);
        void set_side(Side popup_side);

    public:
        virtual void draw(vg::NVGcontext* nvg_context) override;
        virtual void perform_layout(vg::NVGcontext* nvg_context) override;

    protected:
        virtual void refresh_relative_placement() override;

    protected:
        ui::Dialog* m_parent_dialog{ nullptr };
        ds::point<i32> m_anchor_pos{ 0, 0 };
        i32 m_anchor_offset{ 0 };
        i32 m_anchor_size{ 0 };
        ui::Popup::Side m_side{};
    };
}
