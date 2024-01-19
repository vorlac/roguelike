#pragma once

#include "core/ui/button.hpp"
#include "core/ui/popup.hpp"

namespace rl::ui {

    // Button which launches a popup widget.
    //
    // This class overrides ui::widget::m_icon_extra_scale to be 0.8, which
    // affects all subclasses of this ui::widget. Subclasses must explicitly set
    // a different value if needed (e.g., in their constructor).
    class PopupButton : public ui::Button
    {
    public:
        PopupButton(ui::Widget* parent, const std::string& caption = "Untitled",
                    ui::Icon::ID button_icon = ui::Icon::None);

        void set_chevron_icon(ui::Icon::ID icon);
        void set_side(Popup::Side popup_side);

        ui::Popup* popup();
        ui::Icon::ID chevron_icon() const;
        ui::Popup::Side side() const;
        const ui::Popup* popup() const;

    public:
        virtual void draw(vg::NVGcontext* ctx) override;
        virtual ds::dims<f32> preferred_size(vg::NVGcontext* ctx) const override;
        virtual void perform_layout(vg::NVGcontext* ctx) override;

    protected:
        ui::Popup* m_popup{};
        ui::Icon::ID m_chevron_icon{ ui::Icon::None };
    };

}
