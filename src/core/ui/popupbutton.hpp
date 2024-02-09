#pragma once

#include "core/ui/button.hpp"
#include "core/ui/popup.hpp"

namespace rl::ui {

    // Button which launches a popup widget.
    //
    // This class overrides widget::m_icon_extra_scale to be 0.8, which
    // affects all subclasses of this widget. Subclasses must explicitly set
    // a different value if needed (e.g., in their constructor).
    class PopupButton : public Button
    {
    public:
        explicit PopupButton(Widget* parent, std::string caption = "Untitled",
                             Icon::ID button_icon = Icon::None);

        void set_chevron_icon(Icon::ID icon);
        void set_side(Popup::Side popup_side);

        Popup* popup();
        Icon::ID chevron_icon() const;
        Popup::Side side() const;
        const Popup* popup() const;

    public:
        virtual void draw() override;
        virtual ds::dims<f32> preferred_size() const override;
        virtual void perform_layout() override;

    protected:
        Popup* m_popup{ nullptr };
        Icon::ID m_chevron_icon{ Icon::None };
    };

}
