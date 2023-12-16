#include "gui/colorpicker.hpp"
#include "gui/colorwheel.hpp"
#include "gui/layout.hpp"

namespace rl::gui {
    ColorPicker::ColorPicker(Widget* parent, const Color& color)
        : PopupButton(parent, "")
    {
        set_background_color(color);
        Popup& p = this->popup();
        p.set_layout(new GroupLayout());

        mColorWheel = new ColorWheel(&p);
        mPickButton = new Button(&p, "Pick");
        mPickButton->set_fixed_size(Vector2i(100, 25));

        PopupButton::set_changed_callback([&](bool) {
            set_color(background_color());
            m_color_picker_callback(background_color());
        });

        mColorWheel->set_callback([&](const Color& value) {
            mPickButton->set_background_color(value);
            mPickButton->set_text_color(value.contrastingColor());
            m_color_picker_callback(value);
        });

        mPickButton->set_callback([&]() {
            Color value = mColorWheel->color();
            set_pushed(false);
            set_color(value);
            m_color_picker_callback(value);
        });
    }

    Color ColorPicker::color() const
    {
        return background_color();
    }

    void ColorPicker::set_color(const Color& color)
    {
        /* Ignore set_color() calls when the user is currently editing */
        if (!m_pushed)
        {
            Color fg = color.contrastingColor();
            set_background_color(color);
            set_text_color(fg);
            mColorWheel->set_color(color);
            mPickButton->set_background_color(color);
            mPickButton->set_text_color(fg);
        }
    }
}
