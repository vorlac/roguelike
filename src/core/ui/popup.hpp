#pragma once

#include "core/ui/dialog.hpp"
#include "utils/numeric.hpp"

namespace rl::ui {

    class Popup : public Dialog
    {
    public:
        enum class Side {
            Left = 0,
            Right
        };

        Popup(Widget* parent, Dialog* parent_dialog = nullptr);

        f32 anchor_offset() const;
        f32 anchor_size() const;
        Side side() const;
        Dialog* parent_window();
        const Dialog* parent_window() const;
        const ds::point<f32> anchor_pos() const;

        void set_anchor_pos(const ds::point<f32>& anchor_pos);
        void set_anchor_offset(f32 anchor_offset);
        void set_anchor_size(f32 anchor_size);
        void set_side(Side popup_side);

    public:
        virtual void draw() override;
        virtual void perform_layout() override;
        virtual void refresh_relative_placement() override;

    protected:
        Dialog* m_parent_dialog{ nullptr };
        ds::point<f32> m_anchor_pos{ 0.0f, 0.0f };
        f32 m_anchor_offset{ 0.0f };
        f32 m_anchor_size{ 0.0f };
        Popup::Side m_side{ Popup::Side::Left };
    };
}
