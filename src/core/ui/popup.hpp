#pragma once

#include "core/ui/dialog.hpp"
#include "utils/numeric.hpp"

struct NVGcontext;

namespace rl::ui {

    class Popup : public ui::Dialog
    {
    public:
        enum Side {
            Left = 0,
            Right
        };

        Popup(ui::widget* parent, ui::Dialog* parent_window = nullptr);

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
        virtual void draw(NVGcontext* nvg_context) override;
        virtual void perform_layout(NVGcontext* nvg_context) override;

    protected:
        virtual void refresh_relative_placement() override;

    protected:
        ui::Dialog* m_parent_window{ nullptr };
        ds::point<i32> m_anchor_pos{ 0, 0 };
        i32 m_anchor_offset{ 0 };
        i32 m_anchor_size{ 0 };
        Popup::Side m_side{};
    };
}
