#pragma once

#include "gui/checkbox.hpp"

namespace rl::gui {
    /**
     * \class SwitchBox checkbox.h sdlgui/checkbox.h
     *
     * \brief Two-state SwitchBox box widget.
     *
     * \remark
     *     This class overrides \ref nanogui::Widget::mIconExtraScale to be ``1.2f``,
     *     which affects all subclasses of this Widget.  Subclasses must explicitly
     *     set a different value if needed (e.g., in their constructor).
     */
    class SwitchBox : public CheckBox
    {
    public:
        enum class Alignment {
            Horizontal,
            Vertical
        };
        /**
         * Adds a SwitchBox to the specified ``parent``.
         *
         * \param parent
         *     The Widget to add this SwitchBox to.
         *
         * \param caption
         *     The caption text of the SwitchBox (default ``"Untitled"``).
         *
         * \param callback
         *     If provided, the callback to execute when the SwitchBox is checked or
         *     unchecked.  Default parameter function does nothing.  See
         *     \ref nanogui::SwitchBox::mPushed for the difference between "pushed"
         *     and "checked".
         */
        SwitchBox(Widget* parent, Alignment align = Alignment::Horizontal,
                  const std::string& caption = "Untitled",
                  const std::function<void(bool)>& callback = std::function<void(bool)>());

        /// The preferred size of this SwitchBox.
        virtual Vector2i preferredSize(SDL3::SDL_Renderer* renderer) const override;

        /// Draws this SwitchBox.
        virtual void draw(SDL3::SDL_Renderer* renderer) override;
        virtual void drawBody(SDL3::SDL_Renderer* renderer) override;
        virtual void drawKnob(SDL3::SDL_Renderer* renderer);

        virtual void setAlignment(Alignment align)
        {
            mAlign = align;
        }

    protected:
        Alignment mAlign = Alignment::Horizontal;
        float path = 0.f;

        struct AsyncTexture;
        typedef std::shared_ptr<AsyncTexture> AsyncTexturePtr;
        std::vector<AsyncTexturePtr> _txs;
    };
}
