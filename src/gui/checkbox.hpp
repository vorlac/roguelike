#pragma once

#include <memory>
#include <vector>

#include "gui/widget.hpp"

namespace rl::gui {
    class CheckBox : public Widget
    {
    public:
        CheckBox(Widget* parent, const std::string& caption = "Untitled",
                 const std::function<void(bool)>& callback = std::function<void(bool)>());

        const std::string& caption() const
        {
            return m_caption;
        }

        void set_caption(const std::string& caption)
        {
            m_caption = caption;
        }

        const bool& checked() const
        {
            return m_checked;
        }

        void set_checked(const bool& checked)
        {
            m_checked = checked;
        }

        CheckBox& with_checked(bool value)
        {
            set_checked(value);
            return *this;
        }

        const bool& pushed() const
        {
            return m_pushed;
        }

        void set_pushed(const bool& pushed)
        {
            m_pushed = pushed;
        }

        std::function<void(bool)> callback() const
        {
            return m_checkbox_callback;
        }

        void set_callback(const std::function<void(bool)>& callback)
        {
            m_checkbox_callback = callback;
        }

        virtual bool mouse_button_event(const Vector2i& p, int button, bool down,
                                        int modifiers) override;
        Vector2i preferred_size(SDL3::SDL_Renderer* ctx) const override;
        void draw(const std::unique_ptr<rl::Renderer>& renderer) override;
        void draw(SDL3::SDL_Renderer* ctx) override;
        virtual void draw_body(SDL3::SDL_Renderer* renderer);

    protected:
        std::string m_caption{};
        bool m_pushed{};
        bool m_checked{};

        Texture m_caption_texture;
        Texture m_point_texture;

        std::function<void(bool)> m_checkbox_callback;

        struct AsyncTexture;
        std::vector<std::shared_ptr<CheckBox::AsyncTexture>> m_textures;

        std::shared_ptr<CheckBox::AsyncTexture> m_curr_texture{};

    private:
        void draw_texture(std::shared_ptr<CheckBox::AsyncTexture>& texture,
                          SDL3::SDL_Renderer* renderer);
    };
}
