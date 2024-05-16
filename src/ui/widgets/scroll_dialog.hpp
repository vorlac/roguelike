#pragma once

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
            explicit ScrollableDialog(std::string title = {},
                                      ds::dims<f32> fixed_size = {});
            void center();
            void dispose();
            void set_scroll_pos(f32 pos);
            void set_title(std::string title);
            void enable_interaction(Interaction inter);
            void disable_interaction(Interaction inter);

            [[nodiscard]] f32 scroll_pos() const;
            [[nodiscard]] f32 header_height() const;
            [[nodiscard]] std::string_view title() const;

            using interactions_t = std::tuple<Interaction, Component, Side>;
            [[nodiscard]] interactions_t check_interaction(ds::point<f32> pt) const;
            [[nodiscard]] bool interaction_enabled(Interaction inter) const;
            [[nodiscard]] bool mode_active(Interaction inter) const;

        public:
            virtual bool on_mouse_button_pressed(const Mouse& mouse, const Keyboard& kb) override;
            virtual bool on_mouse_button_released(const Mouse& mouse, const Keyboard& kb) override;
            virtual bool on_mouse_scroll(const Mouse& mouse, const Keyboard& kb) override;
            virtual bool on_mouse_drag(const Mouse& mouse, const Keyboard& kb) override;

            virtual void draw() override;
            virtual ds::dims<f32> preferred_size() const override;
            virtual void refresh_relative_placement();

        protected:
            bool m_header_visible{ false };
            bool m_scrollbar_visible{ false };
            f32 m_scrollbar_position{ 0.0f };
            Interaction m_enabled_interactions{ Interaction::All };
            Interaction m_active_interactions{ Interaction::None };
            std::string m_title{};

        private:
            BoxLayout<Alignment::Horizontal>* m_titlebar_layout{ nullptr };
            BoxLayout<Alignment::Horizontal>* m_body_layout{ nullptr };
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
