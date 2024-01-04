#pragma once

#include "core/ui/widget.hpp"

namespace rl::gui {

    class VScrollPanel : public ui::widget
    {
    public:
        VScrollPanel(ui::widget* parent);

        float scroll() const;
        void set_scroll(f32 scroll);

    public:
        virtual void perform_layout(NVGcontext* ctx) override;
        virtual ds::dims<i32> preferred_size(NVGcontext* ctx) const override;
        virtual bool on_mouse_button_pressed(ds::point<i32> pos, Mouse::Button::type btn,
                                             i32 modifiers) override;
        virtual bool on_mouse_button_released(ds::point<i32> pos, Mouse::Button::type btn,
                                              i32 modifiers) override;
        virtual bool on_mouse_drag(ds::point<i32> pos, ds::vector2<i32> rel,
                                   Mouse::Button::type btn, i32 modifiers) override;
        virtual bool on_mouse_scroll(ds::point<i32> pos, ds::vector2<i32> wheel) override;
        virtual void draw(NVGcontext* ctx) override;

    protected:
        i32 m_child_preferred_height{ 0 };
        f32 m_scroll{ 0.0f };
        bool m_update_layout{ false };
    };

}
