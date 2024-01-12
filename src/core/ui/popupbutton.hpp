#pragma once

#include "core/ui/button.hpp"
#include "core/ui/popup.hpp"

namespace rl::ui {

    /// @brief
    ///     Button which launches a popup widget.
    /// @brief
    ///     This class overrides ui::widget::m_icon_extra_scale to be 0.8, which
    ///     affects all subclasses of this ui::widget. Subclasses must explicitly set
    ///     a different value if needed (e.g., in their constructor).
    class PopupButton : public ui::Button
    {
    public:
        PopupButton(ui::widget* parent, const std::string& caption = "Untitled",
                    ui::Icon button_icon = ui::Icon::None);

        void set_chevron_icon(ui::Icon icon);
        void set_side(Popup::Side popup_side);

        Popup* popup();
        ui::Icon chevron_icon() const;
        Popup::Side side() const;
        const ui::Popup* popup() const;

    public:
        virtual void draw(NVGcontext* ctx) override;
        virtual ds::dims<i32> preferred_size(NVGcontext* ctx) const override;
        virtual void perform_layout(NVGcontext* ctx) override;

    protected:
        Popup* m_popup{};
        ui::Icon m_chevron_icon{ ui::Icon::None };
    };

}
