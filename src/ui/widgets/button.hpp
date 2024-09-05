#pragma once

#include <functional>
#include <string_view>
#include <vector>

#include "core/mouse.hpp"
#include "ds/color.hpp"
#include "ds/margin.hpp"
#include "ui/widget.hpp"
#include "utils/numeric.hpp"

namespace rl::ui {
    class PopupButton;

    class Button : public Widget {
    public:
        enum class Property {
            StandardPush = 1 << 0,
            Radio = 1 << 1,
            Toggle = 1 << 2,
            PopupMenu = 1 << 3,
            StandardMenu = 1 << 4,
            Toolbar = Radio | Toggle,
            TogglePopupMenu = PopupMenu | Toggle,
        };

    public:
        explicit Button(Icon::ID icon);
        explicit Button(std::string text, Icon::ID icon = Icon::None);
        explicit Button(Widget* parent, std::string text, Icon::ID icon = Icon::None);

        void set_property(Button::Property prop);
        void set_text(std::string text);
        void set_background_color(ds::color<f32> bg_color);
        void set_text_color(ds::color<f32> text_color);
        void set_icon(Icon::ID icon);
        void set_icon_placement(Icon::Placement placement);
        void set_pressed(bool pressed);
        void set_callback(const std::function<void()>& callback);
        void set_change_callback(const std::function<void(bool)>& callback);
        void set_button_group(const std::vector<Button*>& button_group);

        [[nodiscard]] const std::function<void()>& callback() const;
        [[nodiscard]] const std::function<void(bool)>& change_callback() const;
        [[nodiscard]] const std::vector<Button*>& button_group() const;

        [[nodiscard]] Icon::ID icon() const;
        [[nodiscard]] std::string_view text() const;
        [[nodiscard]] Button::Property properties() const;
        [[nodiscard]] Icon::Placement icon_placement() const;
        [[nodiscard]] ds::color<f32> background_color() const;
        [[nodiscard]] ds::color<f32> text_color() const;
        [[nodiscard]] bool has_property(Button::Property prop) const;
        [[nodiscard]] bool pressed() const;

    public:
        virtual bool on_mouse_entered(const Mouse& mouse) override;
        virtual bool on_mouse_exited(const Mouse& mouse) override;
        virtual bool on_mouse_button_pressed(const Mouse& mouse, const Keyboard& kb, ds::point<f32> local_pos = {}) override;
        virtual bool on_mouse_button_released(const Mouse& mouse, const Keyboard& kb) override;

    public:
        virtual void draw() override;
        virtual ds::dims<f32> preferred_size() const override;

    private:
        bool handle_mouse_button_event(ds::point<f32> pt, Mouse::Button::ID button,
                                       bool button_pressed, Keyboard::Scancode keys_down);

    protected:
        bool m_pressed{ false };
        std::string m_text{};
        Icon::ID m_icon{ Icon::None };
        Button::Property m_props{ Property::StandardPush };
        Icon::Placement m_icon_placement{ Icon::Placement::LeftCentered };
        std::vector<Button*> m_button_group{};
        ds::color<f32> m_background_color{ Colors::Transparent };
        ds::color<f32> m_text_color{ Colors::Transparent };
        std::function<void(bool)> m_change_callback;
        std::function<void()> m_callback;

    private:
        constexpr static inline ds::margin<f32> INNER_PADDING{
            ds::margin<f32>::init(2.5f, 2.5f, 5.0f, 5.0f),
        };
    };
}
