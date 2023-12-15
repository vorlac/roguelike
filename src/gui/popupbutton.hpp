#pragma once

#include "gui/button.hpp"
#include "gui/common.hpp"
#include "gui/entypo.hpp"
#include "gui/popup.hpp"

namespace rl::gui {

    /**
     * \class PopupButton popupbutton.h sdl_gui/popupbutton.h
     *
     * \brief Button which launches a popup widget.
     */
    class PopupButton : public Button
    {
    public:
        PopupButton(Widget* parent, const std::string& caption = "Untitled", int buttonIcon = 0,
                    int chevronIcon = ENTYPO_ICON_CHEVRON_SMALL_RIGHT);

        void setChevronIcon(int icon)
        {
            mChevronIcon = icon;
        }

        int chevronIcon() const
        {
            return mChevronIcon;
        }

        Popup& popup(const Vector2i& size)
        {
            mPopup->setFixedSize(size);
            return *mPopup;
        }

        Popup& popup()
        {
            return *mPopup;
        }

        Popup* popupptr()
        {
            return mPopup;
        }

        void draw(SDL3::SDL_Renderer* renderer) override;
        Vector2i preferredSize(SDL3::SDL_Renderer* ctx) const override;
        void performLayout(SDL3::SDL_Renderer* ctx) override;

        PopupButton& withChevron(int icon)
        {
            setChevronIcon(icon);
            return *this;
        }

    protected:
        Popup* mPopup = nullptr;
        int mChevronIcon;

        Texture _chevronTex;
    };
}