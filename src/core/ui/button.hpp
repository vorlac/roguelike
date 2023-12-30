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

    /// @brief
    ///     [Normal/Toggle/Radio/Popup] Button ui::widget.
    class Button : public ui::widget
    {
    public:
        // Flags to specify the button behavior
        enum Flags {
            NormalButton = 1 << 0,  // normal button
            RadioButton = 1 << 1,   // radio button
            ToggleButton = 1 << 2,  // toggle button
            PopupButton = 1 << 3,   // popup button
            MenuButton = 1 << 4     // menu button
        };

        // The available icon positions.
        enum class IconPosition {
            Left,           // far left.
            LeftCentered,   // left, centered (depends on caption text length).
            RightCentered,  // right, centered (depends on caption text length).
            Right           // far right.
        };

        Button(ui::widget* parent, const std::string& caption = "Untitled", i32 icon = 0);

        i32 icon() const;
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
        void set_background_color(const ds::color<u8>& background_color);
        void set_text_color(const ds::color<u8>& text_color);
        void set_icon(i32 icon);
        void set_flags(Button::Flags button_flags);
        void set_icon_position(IconPosition icon_position);
        void set_pressed(bool pressed);
        void set_callback(const std::function<void()>& callback);
        void set_change_callback(const std::function<void(bool)>& callback);
        void set_button_group(const std::vector<Button*>& button_group);

    public:
        virtual void draw(NVGcontext* ctx) override;
        virtual ds::dims<i32> preferred_size(NVGcontext* ctx) const override;
        virtual bool on_mouse_enter(const ds::point<i32>& pt) override;
        virtual bool on_mouse_leave(const ds::point<i32>& pt) override;
        virtual bool on_mouse_click(const ds::point<i32>& pt, rl::Mouse::Button::ID button,
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
        i32 m_icon{};
        Button::IconPosition m_icon_position{};
        bool m_pressed{ false };
        Button::Flags m_flags{};
        ds::color<u8> m_background_color{};
        ds::color<u8> m_text_color{};
        std::function<void()> m_callback;
        std::function<void(bool)> m_change_callback;
        std::vector<ui::Button*> m_button_group{};
    };
}
