#pragma once

#include <functional>
#include <memory>
#include <string>

#include "gui/common.hpp"
#include "gui/widget.hpp"

#pragma warning(disable : 4244)

namespace rl::gui {

    /**
     * \class Button button.h sdlgui/button.h
     *
     * \brief [Normal/Toggle/Radio/Popup] Button widget.
     */
    class Button : public Widget
    {
    public:
        /// Flags to specify the button behavior (can be combined with binary OR)
        enum Flags {
            NormalButton = (1 << 0),  // 1
            RadioButton = (1 << 1),   // 2
            ToggleButton = (1 << 2),  // 4
            PopupButton = (1 << 3)    // 8
        };

        /// The available icon positions.
        enum class IconPosition {
            Left,
            LeftCentered,
            RightCentered,
            Right
        };

        Button(Widget* parent, const std::string& caption = "Untitled", int icon = 0);

        Button(Widget* parent, const std::string& caption, const std::function<void()>& callback)
            : Button(parent, caption)
        {
            setCallback(callback);
        }

        Button(Widget* parent, const std::string& caption, int icon,
               const std::function<void()>& callback)
            : Button(parent, caption, icon)
        {
            setCallback(callback);
        }

        Button(Widget* parent, const std::string& caption,
               const std::function<void(bool state)>& callback)
            : Button(parent, caption)
        {
            setChangeCallback(callback);
        }

        const std::string& caption() const
        {
            return mCaption;
        }

        void setCaption(const std::string& caption)
        {
            mCaption = caption;
            _captionTex.dirty = true;
        }

        const Color& backgroundColor() const
        {
            return mBackgroundColor;
        }

        void setBackgroundColor(const Color& backgroundColor)
        {
            mBackgroundColor = backgroundColor;
        }

        const Color& textColor() const
        {
            return mTextColor;
        }

        void setTextColor(const Color& textColor);

        int icon() const
        {
            return static_cast<int>(mIcon);
        }

        void setIcon(int icon)
        {
            mIcon = icon;
        }

        int flags() const
        {
            return mFlags;
        }

        void setFlags(int buttonFlags)
        {
            mFlags = buttonFlags;
        }

        IconPosition iconPosition() const
        {
            return mIconPosition;
        }

        void setIconPosition(IconPosition iconPosition)
        {
            mIconPosition = iconPosition;
        }

        bool pushed() const
        {
            return mPushed;
        }

        void setPushed(bool pushed)
        {
            mPushed = pushed;
        }

        /// Set the push callback (for any type of button)
        std::function<void()> callback() const
        {
            return m_pressed_callback;
        }

        void setCallback(const std::function<void()>& callback)
        {
            m_pressed_callback = callback;
        }

        /// Set the change callback (for toggle buttons)
        std::function<void(bool)> changeCallback() const
        {
            return m_change_callback;
        }

        void setChangeCallback(const std::function<void(bool)>& callback)
        {
            m_change_callback = callback;
        }

        /// Set the button group (for radio buttons)
        void setButtonGroup(const std::vector<Button*>& buttonGroup)
        {
            mButtonGroup = buttonGroup;
        }

        const std::vector<Button*>& buttonGroup() const
        {
            return mButtonGroup;
        }

        virtual Vector2i preferredSize(SDL3::SDL_Renderer* ctx) const override;
        virtual bool mouseButtonEvent(const Vector2i& p, int button, bool down,
                                      int modifiers) override;
        virtual void draw(SDL3::SDL_Renderer* renderer) override;
        virtual void drawBody(SDL3::SDL_Renderer* renderer);
        virtual void drawBodyTemp(SDL3::SDL_Renderer* renderer);
        virtual Color bodyColor();
        virtual Vector2i getTextOffset() const;

        Button& withCallback(const std::function<void()>& callback)
        {
            setCallback(callback);
            return *this;
        }

        Button& withFlags(int flags)
        {
            setFlags(flags);
            return *this;
        }

        Button& withChangeCallback(const std::function<void(bool)>& callback)
        {
            setChangeCallback(callback);
            return *this;
        }

        Button& withBackgroundColor(const Color& color)
        {
            setBackgroundColor(color);
            return *this;
        }

        Button& withIcon(int icon)
        {
            setIcon(icon);
            return *this;
        }

    protected:
        virtual void renderBodyTexture(NVGcontext*& ctx, int& realw, int& realh);

        std::string mCaption;
        intptr_t mIcon;
        IconPosition mIconPosition;
        bool mPushed;
        int mFlags;
        Color mBackgroundColor;
        Color mTextColor;

        Texture _captionTex;
        Texture _iconTex;

        std::function<void()> m_pressed_callback;
        std::function<void(bool)> m_change_callback;
        std::vector<Button*> mButtonGroup;

        struct AsyncTexture;
        typedef std::shared_ptr<AsyncTexture> AsyncTexturePtr;
        std::vector<AsyncTexturePtr> _txs;

        AsyncTexturePtr current_texture_ = nullptr;

    private:
        void drawTexture(AsyncTexturePtr& texture, SDL3::SDL_Renderer* renderer);
    };

}
