#pragma once

#include <vector>

#include "gui/screen.hpp"
#include "gui/window.hpp"

namespace rl::gui {
    /**
     * \class Popup popup.h sdl_gui/popup.h
     *
     * \brief Popup window for combo boxes, popup buttons, nested dialogs etc.
     *
     * Usually the Popup instance is constructed by another widget (e.g. \ref PopupButton)
     * and does not need to be created by hand.
     */
    class Popup : public Window
    {
    public:
        /// Create a new popup parented to a screen (first argument) and a parent window
        Popup(Widget* parent, Window* parentWindow);

        /// Return the anchor position in the parent window; the placement of the popup is relative
        /// to it
        void setAnchorPos(const Vector2i& anchorPos)
        {
            mAnchorPos = anchorPos;
        }

        /// Set the anchor position in the parent window; the placement of the popup is relative to
        /// it
        const Vector2i& anchorPos() const
        {
            return mAnchorPos;
        }

        /// Set the anchor height; this determines the vertical shift relative to the anchor
        /// position
        void setAnchorHeight(int anchorHeight)
        {
            mAnchorHeight = anchorHeight;
        }

        /// Return the anchor height; this determines the vertical shift relative to the anchor
        /// position
        int anchorHeight() const
        {
            return mAnchorHeight;
        }

        /// Return the parent window of the popup
        Window* parentWindow()
        {
            return mParentWindow;
        }

        /// Return the parent window of the popup
        const Window* parentWindow() const
        {
            return mParentWindow;
        }

        /// Invoke the associated layout generator to properly place child widgets, if any
        void perform_layout(SDL3::SDL_Renderer* ctx) override;

        /// Draw the popup window
        void draw(SDL3::SDL_Renderer* renderer) override;
        virtual void draw_body(SDL3::SDL_Renderer* renderer) override;
        virtual void draw_body_temp(SDL3::SDL_Renderer* renderer) override;

    protected:
        /// Internal helper function to maintain nested window position values
        virtual void refresh_relative_placement();
        virtual void rendereBodyTexture(NVGcontext*& ctx, int& ctxw, int& ctxh, int dx);
        virtual Vector2i getOverrideBodyPos();

        Window* mParentWindow;
        Vector2i mAnchorPos;
        int mAnchorHeight;
        int _anchorDx = 15;

        struct AsyncTexture;
        typedef std::shared_ptr<Popup::AsyncTexture> AsyncTexturePtr;
        std::vector<Popup::AsyncTexturePtr> m_popup_txs;
    };
}
