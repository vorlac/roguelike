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
            set_callback(callback);
        }

        Button(Widget* parent, const std::string& caption, int icon,
               const std::function<void()>& callback)
            : Button(parent, caption, icon)
        {
            set_callback(callback);
        }

        Button(Widget* parent, const std::string& caption,
               const std::function<void(bool state)>& callback)
            : Button(parent, caption)
        {
            set_changed_callback(callback);
        }

        const std::string& caption() const
        {
            return m_caption;
        }

        void set_caption(const std::string& caption)
        {
            m_caption = caption;
            m_caption_texture.dirty = true;
        }

        const Color& background_color() const
        {
            return m_background_color;
        }

        void set_background_color(const Color& background_color)
        {
            m_background_color = background_color;
        }

        const Color& text_color() const
        {
            return m_text_color;
        }

        void set_text_color(const Color& text_color);

        int icon() const
        {
            return static_cast<int>(m_icon);
        }

        void set_icon(int icon)
        {
            m_icon = icon;
        }

        int flags() const
        {
            return m_flags;
        }

        void set_flags(int buttonFlags)
        {
            m_flags = buttonFlags;
        }

        IconPosition iconPosition() const
        {
            return m_icon_position;
        }

        void set_icon_position(IconPosition iconPosition)
        {
            m_icon_position = iconPosition;
        }

        bool pushed() const
        {
            return m_pushed;
        }

        void set_pushed(bool pushed)
        {
            m_pushed = pushed;
        }

        /// Set the push callback (for any type of button)
        std::function<void()> callback() const
        {
            return m_pressed_callback;
        }

        void set_callback(const std::function<void()>& callback)
        {
            m_pressed_callback = callback;
        }

        /// Set the change callback (for toggle buttons)
        std::function<void(bool)> change_callback() const
        {
            return m_change_callback;
        }

        void set_changed_callback(const std::function<void(bool)>& callback)
        {
            m_change_callback = callback;
        }

        /// Set the button group (for radio buttons)
        void set_button_group(const std::vector<Button*>& button_group)
        {
            m_button_group = button_group;
        }

        const std::vector<Button*>& button_group() const
        {
            return m_button_group;
        }

        virtual Vector2i preferred_size(SDL3::SDL_Renderer* ctx) const override;
        virtual bool mouse_button_event(const Vector2i& p, int button, bool down,
                                        int modifiers) override;
        virtual void draw(SDL3::SDL_Renderer* renderer) override;
        virtual void draw_body(SDL3::SDL_Renderer* renderer);
        virtual void draw_body_temp(SDL3::SDL_Renderer* renderer);
        virtual Color body_color();
        virtual Vector2i get_text_offset() const;

        Button& with_callback(const std::function<void()>& callback)
        {
            set_callback(callback);
            return *this;
        }

        Button& with_flags(int flags)
        {
            set_flags(flags);
            return *this;
        }

        Button& with_change_callback(const std::function<void(bool)>& callback)
        {
            set_changed_callback(callback);
            return *this;
        }

        Button& with_background_color(const Color& color)
        {
            set_background_color(color);
            return *this;
        }

        Button& with_icon(int icon)
        {
            set_icon(icon);
            return *this;
        }

    protected:
        virtual void render_body_texture(NVGcontext*& ctx, int& realw, int& realh);

    protected:
        struct AsyncTexture;
        using AsyncTexturePtr = std::shared_ptr<Button::AsyncTexture>;

        int m_flags{ 0 };
        bool m_pushed{ false };

        Color m_text_color{};
        Color m_background_color{};
        Texture m_icon_texture{};
        Texture m_caption_texture{};
        IconPosition m_icon_position{};
        std::string m_caption{};
        intptr_t m_icon{};

        std::function<void()> m_pressed_callback{};
        std::function<void(bool)> m_change_callback{};
        std::vector<Button*> m_button_group{};
        std::vector<Button::AsyncTexturePtr> m_textures{};
        Button::AsyncTexturePtr m_curr_texture{ nullptr };

    private:
        void draw_texture(Button::AsyncTexturePtr& texture, SDL3::SDL_Renderer* renderer);
    };
}
