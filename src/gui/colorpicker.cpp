#include "gui/colorpicker.hpp"
#include "gui/colorwheel.hpp"
#include "gui/layout.hpp"

namespace rl::gui {
    ColorPicker::ColorPicker(Widget* parent, const Color& color)
        : PopupButton(parent, "")
    {
        setBackgroundColor(color);
        Popup& p = this->popup();
        p.set_layout(new GroupLayout());

        mColorWheel = new ColorWheel(&p);
        mPickButton = new Button(&p, "Pick");
        mPickButton->set_fixed_size(Vector2i(100, 25));

        PopupButton::setChangeCallback([&](bool) {
            setColor(backgroundColor());
            m_color_picker_callback(backgroundColor());
        });

        mColorWheel->setCallback([&](const Color& value) {
            mPickButton->setBackgroundColor(value);
            mPickButton->setTextColor(value.contrastingColor());
            m_color_picker_callback(value);
        });

        mPickButton->setCallback([&]() {
            Color value = mColorWheel->color();
            setPushed(false);
            setColor(value);
            m_color_picker_callback(value);
        });
    }

    Color ColorPicker::color() const
    {
        return backgroundColor();
    }

    void ColorPicker::setColor(const Color& color)
    {
        /* Ignore setColor() calls when the user is currently editing */
        if (!mPushed)
        {
            Color fg = color.contrastingColor();
            setBackgroundColor(color);
            setTextColor(fg);
            mColorWheel->setColor(color);
            mPickButton->setBackgroundColor(color);
            mPickButton->setTextColor(fg);
        }
    }
}
