#pragma once

#include "core/ui/widget.hpp"

namespace rl::ui {

    class VScrollPanel final : public Widget
    {
    public:
        explicit VScrollPanel(Widget* parent);

        f32 scroll() const;
        void set_scroll(f32 scroll);

    public:
        virtual bool on_mouse_button_pressed(const Mouse& mouse, const Keyboard& kb) override;
        virtual bool on_mouse_button_released(const Mouse& mouse, const Keyboard& kb) override;
        virtual bool on_mouse_scroll(const Mouse& mouse, const Keyboard& kb) override;
        virtual bool on_mouse_drag(const Mouse& mouse, const Keyboard& kb) override;

    public:
        virtual void draw() override;
        virtual void perform_layout() override;
        virtual ds::dims<f32> preferred_size() const override;

    protected:
        f32 m_child_preferred_height{ 0.0f };
        f32 m_scroll{ 0.0f };
        bool m_update_layout{ false };
    };

}
