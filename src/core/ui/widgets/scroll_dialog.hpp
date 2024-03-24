#pragma once

#include <memory>
#include <string>
#include <tuple>
#include <utility>

#include "core/ui/widget.hpp"
#include "ds/color.hpp"

namespace rl {
    class Keyboard;
    class Mouse;

    namespace ui {

        class ScrollableDialog : public Widget
        {
        public:
            template <typename TContainer = std::vector<std::vector<Widget*>>>
            ScrollableDialog(Widget* parent, std::string title, TContainer&& widgets = {})
                : Widget{ parent }
                , m_title{ std::move(title) }
            {
                if (!widgets.empty())
                    m_layout->set_widget_table(this, std::forward<TContainer>(widgets));
            }

            std::tuple<Interaction, Component, Side> check_interaction(
                const ds::point<f32>& pt) const;

            void center();
            void dispose();
            void set_title(std::string title);
            void enable_interaction(Interaction inter);
            void disable_interaction(Interaction inter) const;
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
            virtual void perform_layout() override;
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
            constexpr static inline ds::color<f32> SDScrollbarColor{ 220, 220, 220, 100 };
            constexpr static inline ds::color<f32> SDScrollbarShadowColor{ 128, 128, 128, 100 };
            constexpr static inline ds::color<f32> SDScrollGuideColor{ 0, 0, 0, 32 };
            constexpr static inline ds::color<f32> SDScrollGuideShadowColor{ 0, 0, 0, 92 };

            constexpr static inline f32 SDScrollBarBackgroundRadius{ 3.0f };
            constexpr static inline f32 SDScrollBarCornerRadius{ 2.0f };
            constexpr static inline f32 SDShadowBlur{ 4.0f };
            constexpr static inline f32 SDScrollbarWidth{ 12.0f };
            constexpr static inline f32 SDScrollbarBorder{ 1.0f };
            constexpr static inline f32 SDOutlineSize{ 1.0f };
            constexpr static inline f32 SDMargin{ 4.0f };
        };
    }
}
