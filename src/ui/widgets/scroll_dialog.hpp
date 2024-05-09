#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>

#include "ds/color.hpp"
#include "ui/layouts/box_layout.hpp"
#include "ui/widgets/Label.hpp"
#include "ui/widgets/button.hpp"

namespace rl {
    class Keyboard;
    class Mouse;

    namespace ui {

        class ScrollableDialog : public Widget
        {
        public:
            explicit ScrollableDialog(std::string title = "", const ds::dims<f32> fixed_size = ds::dims<f32>::zero())
                : Widget{ nullptr }
                , m_title{ std::move(title) }
            {
                if (fixed_size.valid())
                    Widget::set_size(fixed_size);

                const auto min_btn{ new ui::Button{ Icon::WindowMinimize } };
                const auto max_btn{ new ui::Button{ Icon::WindowMaximize } };
                const auto cls_btn{ new ui::Button{ Icon::WindowClose } };

                min_btn->set_font_size(18.0f);
                min_btn->set_fixed_height(18.0f);
                max_btn->set_font_size(18.0f);
                max_btn->set_fixed_height(18.0f);
                cls_btn->set_font_size(18.0f);
                cls_btn->set_fixed_height(18.0f);

                // horizontally aligns title (centered), minimize, maximize, and close buttons
                const auto titlebar_layout{ new BoxLayout<Alignment::Horizontal>{ "Titlebar Horiz" } };
                titlebar_layout->set_margins({ 2.0f }, { 2.0f });

                titlebar_layout->set_size_policy(SizePolicy::Minimum);
                titlebar_layout->add_widget(min_btn);
                titlebar_layout->add_widget(max_btn);
                titlebar_layout->add_widget(cls_btn);

                // horizontally aligns the contents panel containing all children, and scrollbar
                const auto body_label{ new ui::Label{ "Body", -1.0f, Align::HCenter | Align::VMiddle } };
                const auto body_layout{ new BoxLayout<Alignment::Horizontal>{ "Body Horiz" } };
                body_layout->add_widget(body_label);
                body_layout->set_margins({ 2.0f }, { 2.0f });

                // vertically aligns titlebar and dialog body
                const auto root_layout{ new BoxLayout<Alignment::Vertical>{ "Dialog Root Vert" } };
                root_layout->set_margins({ 2.0f }, { 2.0f });

                root_layout->set_size_policy(SizePolicy::Maximum);
                root_layout->add_nested_layout(titlebar_layout);
                root_layout->add_nested_layout(body_layout);

                this->assign_layout(root_layout);
                Widget::perform_layout();
            }

            [[nodiscard]]
            std::tuple<Interaction, Component, Side> check_interaction(ds::point<f32> pt) const;

            void center();
            void dispose();
            void set_title(std::string title);
            void enable_interaction(Interaction inter);
            void disable_interaction(Interaction inter);
            bool interaction_enabled(Interaction inter) const;
            bool mode_active(Interaction inter) const;

            f32 scroll_pos() const;
            f32 header_height() const;
            std::string title() const;
            Widget* button_panel() const;
            void set_scroll_pos(f32 pos);

        public:
            virtual bool on_mouse_button_pressed(const Mouse& mouse, const Keyboard& kb) override;
            virtual bool on_mouse_button_released(const Mouse& mouse, const Keyboard& kb) override;
            virtual bool on_mouse_scroll(const Mouse& mouse, const Keyboard& kb) override;
            virtual bool on_mouse_drag(const Mouse& mouse, const Keyboard& kb) override;

            virtual void draw() override;
            virtual void refresh_relative_placement();
            virtual ds::dims<f32> preferred_size() const override;

        protected:
            bool m_header_visible{ false };
            bool m_scrollbar_visible{ false };
            f32 m_scrollbar_position{ 0.0f };
            Widget* m_button_panel{ nullptr };
            Interaction m_enabled_interactions{ Interaction::All };
            Interaction m_active_interactions{ Interaction::None };
            std::string m_title{};

        private:
            constexpr static ds::color<f32> SDScrollbarColor{ 220, 220, 220, 100 };
            constexpr static ds::color<f32> SDScrollbarShadowColor{ 128, 128, 128, 100 };
            constexpr static ds::color<f32> SDScrollGuideColor{ 0, 0, 0, 32 };
            constexpr static ds::color<f32> SDScrollGuideShadowColor{ 0, 0, 0, 92 };

            constexpr static f32 SDScrollBarBackgroundRadius{ 3.0f };
            constexpr static f32 SDScrollBarCornerRadius{ 2.0f };
            constexpr static f32 SDShadowBlur{ 4.0f };
            constexpr static f32 SDScrollbarWidth{ 12.0f };
            constexpr static f32 SDScrollbarBorder{ 1.0f };
            constexpr static f32 SDOutlineSize{ 1.0f };
            constexpr static f32 SDMargin{ 4.0f };
        };
    }
}
