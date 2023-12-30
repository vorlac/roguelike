#pragma once

#include "core/window.hpp"
#include "utils/numeric.hpp"

struct NVGcontext;

namespace rl::ui {

    /// @brief
    ///     Popup window for combo boxes, popup buttons, nested dialogs etc.
    /// @brief
    ///     Usually the Popup instance is constructed by another widget (e.g.
    ///     PopupButton) and does not need to be created by hand.
    class Popup : public rl::Window
    {
    public:
        enum Side {
            Left = 0,
            Right
        };

        Popup(ui::widget* parent, rl::Window* parent_window = nullptr);

        i32 anchor_offset() const;
        i32 anchor_size() const;
        Side side() const;
        rl::Window* parent_window();
        const rl::Window* parent_window() const;
        const ds::point<i32>& anchor_pos() const;

        void set_anchor_pos(const ds::point<i32>& anchor_pos);
        void set_anchor_offset(i32 anchor_offset);
        void set_anchor_size(i32 anchor_size);
        void set_side(Side popup_side);

    public:
        virtual void perform_layout(NVGcontext* ctx) override;
        virtual void draw(NVGcontext* ctx) override;

    protected:
        virtual void refresh_relative_placement() override;

    protected:
        rl::Window* m_parent_window{ nullptr };
        ds::point<i32> m_anchor_pos{ 0, 0 };
        i32 m_anchor_offset{ 0 };
        i32 m_anchor_size{ 0 };
        Popup::Side m_side{};
    };
}
