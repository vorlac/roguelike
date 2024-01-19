#pragma once

#include <functional>
#include <string>
#include <vector>

#include "core/mouse.hpp"
#include "core/ui/widget.hpp"
#include "ds/color.hpp"
#include "ds/dims.hpp"
#include "ds/point.hpp"
#include "ds/shared.hpp"
#include "utils/numeric.hpp"

namespace rl::ui {

    class Button : public ui::Widget
    {
    public:
        enum Flags {
            NormalButton = 1 << 0,
            RadioButton = 1 << 1,
            ToggleButton = 1 << 2,
            PopupButton = 1 << 3,
            MenuButton = 1 << 4,

            ToolButton = RadioButton | ToggleButton,
        };

    public:
        Button(ui::Widget* parent, const std::string& caption = "Untitled",
               ui::Icon::ID icon = ui::Icon::None);

        bool pressed() const;
        ui::Icon::ID icon() const;
        ui::Button::Flags flags() const;
        ui::Icon::Position icon_position() const;
        ds::color<f32> background_color() const;
        ds::color<f32> text_color() const;
        const std::string& caption() const;
        const std::function<void()>& callback() const;
        const std::function<void(bool)>& change_callback() const;
        const std::vector<Button*>& button_group() const;

        void set_caption(const std::string& caption);
        void set_background_color(ds::color<f32> background_color);
        void set_text_color(ds::color<f32> text_color);
        void set_icon(ui::Icon::ID icon);
        void set_flags(Button::Flags button_flags);
        void set_icon_position(ui::Icon::Position icon_position);
        void set_pressed(bool pressed);
        void set_callback(const std::function<void()>& callback);
        void set_change_callback(const std::function<void(bool)>& callback);
        void set_button_group(const std::vector<Button*>& button_group);

    public:
        virtual bool on_mouse_entered(const Mouse& mouse) override;
        virtual bool on_mouse_exited(const Mouse& mouse) override;
        virtual bool on_mouse_button_pressed(const Mouse& mouse, const Keyboard& kb) override;
        virtual bool on_mouse_button_released(const Mouse& mouse, const Keyboard& kb) override;

    public:
        virtual void draw(vg::NVGcontext* ctx) override;
        virtual ds::dims<f32> preferred_size(vg::NVGcontext* ctx) const override;

    private:
        bool handle_mouse_button_event(const ds::point<i32>& pt, Mouse::Button::ID button,
                                       bool down, Keyboard::Scancode::ID modifiers);

    protected:
        std::string m_caption{};

        // The icon of this Button (0 means no icon).
        // The icon to display with this Button (0 means no icons).
        // If not 0, may either be a picture icon, or one of the icons enumerated in.
        // The kind of icon (image or Entypo) is determined by the functions nvgIsImageIcon and
        // its reciprocal counterpart nvgIsFontIcon.
        ui::Icon::ID m_icon{ ui::Icon::None };
        Button::Flags m_flags{};
        bool m_pressed{ false };
        ui::Icon::Position m_icon_position{};
        ds::color<f32> m_background_color{ rl::Colors::LightGrey };
        ds::color<f32> m_text_color{ rl::Colors::White };
        std::function<void()> m_callback;
        std::function<void(bool)> m_change_callback;
        std::vector<ui::Button*> m_button_group{};
    };
}
