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
        class ScrollableContainer final : public Widget
        {
        public:
            using Widget::Widget;

            virtual bool on_mouse_drag(const Mouse& mouse, const Keyboard& kb) override
            {
                return this->parent()->on_mouse_drag(mouse, kb);
            }
        };

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

    public:
        virtual bool on_mouse_button_pressed(const Mouse& mouse, const Keyboard& kb) override;
        virtual bool on_mouse_button_released(const Mouse& mouse, const Keyboard& kb) override;
        virtual bool on_mouse_move(const Mouse& mouse, const Keyboard& kb) override;
        virtual bool on_mouse_scroll(const Mouse& mouse, const Keyboard& kb) override;
        virtual bool on_mouse_drag(const Mouse& mouse, const Keyboard& kb) override;

        virtual void draw() override;
        virtual bool draw_mouse_intersection(const ds::point<f32>& pt) override;
        virtual void add_child(Widget* child) override;
        virtual void perform_layout() override;

        virtual Widget* find_widget(const ds::point<f32>& pt) override;
        virtual ds::dims<f32> preferred_size() const override;

    protected:
        ScrollableContainer* m_container{ new ScrollableContainer{ nullptr } };
        ds::rect<f32> m_scroll_bar_rect{ ds::point{ 0.0f, 0.0f }, ds::dims{ 0.0f, 0.0f } };
        ds::dims<f32> m_cont_prefsize{ 0.0f, 0.0f };
        ScrollMode m_scrollmode{ ScrollMode::ScrollbarOnly };
        Component m_prev_click_location{ Component::None };
        f32 m_scrollbar_pos{ 0.0f };
        bool m_update_layout{ false };

        constexpr static inline ds::color<f32> ScrollbarColor{ 220, 220, 220, 100 };
        constexpr static inline ds::color<f32> ScrollbarShadowColor{ 128, 128, 128, 100 };
        constexpr static inline ds::color<f32> ScrollGuideColor{ 0, 0, 0, 32 };
        constexpr static inline ds::color<f32> ScrollGuideShadowColor{ 0, 0, 0, 92 };

        constexpr static inline f32 ScrollBarBackgroundRadius{ 3.0f };
        constexpr static inline f32 ScrollBarCornerRadius{ 2.0f };
        constexpr static inline f32 ShadowBlur{ 4.0f };
        constexpr static inline f32 ScrollbarWidth{ 12.0f };
        constexpr static inline f32 ScrollbarBorder{ 1.0f };
        constexpr static inline f32 OutlineSize{ 1.0f };
        constexpr static inline f32 Margin{ 4.0f };
    };

}
