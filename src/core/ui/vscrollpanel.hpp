#pragma once

#include "core/ui/widget.hpp"

namespace rl::gui {

    class VScrollPanel : public ui::Widget
    {
    public:
        VScrollPanel(ui::Widget* parent);

        float scroll() const;
        void set_scroll(f32 scroll);

    public:
        virtual bool on_mouse_button_pressed(const Mouse& mouse, const Keyboard& kb) override;
        virtual bool on_mouse_button_released(const Mouse& mouse, const Keyboard& kb) override;
        virtual bool on_mouse_scroll(const Mouse& mouse, const Keyboard& kb) override;
        virtual bool on_mouse_drag(const Mouse& mouse, const Keyboard& kb) override;

    public:
        virtual void draw(NVGcontext* ctx) override;
        virtual void perform_layout(NVGcontext* ctx) override;
        virtual ds::dims<i32> preferred_size(NVGcontext* ctx) const override;

    protected:
        i32 m_child_preferred_height{ 0 };
        f32 m_scroll{ 0.0f };
        bool m_update_layout{ false };
    };

}
