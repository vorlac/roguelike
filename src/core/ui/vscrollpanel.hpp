#pragma once

#include "core/ui/widget.hpp"

namespace rl::ui {

    enum class ScrollMode {
        ScrollbarOnly,
        BodyOnly,
        Any
    };

    class VScrollPanel final : public Widget
    {
        enum class Component {
            None,
            Body,
            ScrollBar
        };

    public:
        explicit VScrollPanel(Widget* parent);

        f32 scroll() const;
        void set_scroll(f32 scroll);
        Widget* container() const;

        virtual void add_child(Widget* child) override
        {
            runtime_assert(m_container != nullptr, "vscroll container not set up yet");
            m_container->add_child(child);
        }

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
        Widget* m_container{ nullptr };
        f32 m_scroll{ 0.0f };
        bool m_update_layout{ false };

        ScrollMode m_scrollmode{ ScrollMode::ScrollbarOnly };
        Component m_prev_click_location{ Component::None };

        constexpr static f32 CORNER_RADIUS{ 3.0f };
        constexpr static f32 OUTER_SHADOW_BLUR{ 4.0f };
        constexpr static f32 SCROLLBAR_WIDTH{ 8.0f };
        constexpr static f32 OUTER_MARGIN{ 4.0f };
        constexpr static f32 SCROLLBAR_BORDER{ 1.0f };
        constexpr static f32 OUTLINE_SIZE{ 1.0f };
    };

}
