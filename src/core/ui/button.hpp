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
            MenuButton = 1 << 4
        };

        enum class IconPosition {
            Left,           // far left
            LeftCentered,   // left, centered (depends on caption text length)
            RightCentered,  // right, centered (depends on caption text length)
            Right           // far right
        };

    public:
        Button(ui::Widget* parent, const std::string& caption = "Untitled",
               ui::Icon icon = ui::Icon::None);

        ui::Icon icon() const;
        i32 flags() const;
        bool pressed() const;
        IconPosition icon_position() const;
        const std::string& caption() const;
        const ds::color<u8>& background_color() const;
        const ds::color<u8>& text_color() const;
        const std::function<void()>& callback() const;
        const std::function<void(bool)>& change_callback() const;
        const std::vector<Button*>& button_group() const;

        void set_caption(const std::string& caption);
        void set_background_color(ds::color<u8> background_color);
        void set_text_color(ds::color<u8> text_color);
        void set_icon(ui::Icon icon);
        void set_flags(Button::Flags button_flags);
        void set_icon_position(IconPosition icon_position);
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
        virtual void draw(NVGcontext* ctx) override;
        virtual ds::dims<i32> preferred_size(NVGcontext* ctx) const override;

    private:
        bool handle_mouse_button_event(const ds::point<i32>& pt, Mouse::Button::type button,
                                       bool down, i32 modifiers);

    protected:
        std::string m_caption{};

        /// @brief
        ///     The icon of this Button (0 means no icon).
        ///
        ///     The icon to display with this Button. If not 0, may either be a
        ///     picture icon, or one of the icons enumerated in. The kind of icon
        ///     (image or Entypo) is determined by the functions nvgIsImageIcon and
        ///     its reciprocal counterpart nvgIsFontIcon.
        ui::Icon m_icon{ ui::Icon::None };
        Button::Flags m_flags{};
        bool m_pressed{ false };
        Button::IconPosition m_icon_position{};
        ds::color<u8> m_background_color{};
        ds::color<u8> m_text_color{ 255, 255, 255 };
        std::function<void()> m_callback;
        std::function<void(bool)> m_change_callback;
        std::vector<ui::Button*> m_button_group{};
    };
}
