#pragma once

#include <memory>

#include "gui/widget.hpp"
#include "utils/numeric.hpp"

namespace rl::gui {

    /**
     * @brief Top-level window widget.
     */
    class Window : public Widget
    {
        friend class Popup;

    public:
        Window(Widget* parent, const std::string& title = "Untitled");

        Window(Widget* parent, const std::string& title, const Vector2i& pos)
            : Window(parent, title)
        {
            this->set_relative_position(pos);
        }

        // Return the window title
        const std::string& title() const
        {
            return m_title;
        }

        // Set the window title
        void set_title(const std::string& title)
        {
            m_title = title;
        }

        // Is this a model dialog?
        bool modal() const
        {
            return m_modal;
        }

        // Set whether or not this is a modal dialog
        void set_modal(bool modal)
        {
            m_modal = modal;
        }

        // Is this draggable?
        bool draggable() const
        {
            return m_draggable;
        }

        // Set whether or not this is draggable
        void set_draggable(bool draggable)
        {
            m_draggable = draggable;
        }

        // Drop shadow enabled?
        bool drop_shadow_enabled() const
        {
            return m_drop_shadow_enabled;
        }

        // Set whether or not drop shadow is enabled
        void set_drop_shadow_enabled(bool drop_shadow_enabled)
        {
            m_drop_shadow_enabled = drop_shadow_enabled;
        }

        // Return the panel used to house window buttons
        Widget* button_panel();

        // Dispose the window
        void dispose();

        // Center the window in the current \ref Screen
        void center();

        // Draw the window
        void draw(SDL3::SDL_Renderer* surface) override;
        virtual void draw_body(SDL3::SDL_Renderer* renderer);
        virtual void draw_body_temp(SDL3::SDL_Renderer* renderer);

        // Handle window drag events
        bool mouse_drag_event(const Vector2i& p, const Vector2i& rel, i32 button,
                              i32 modifiers) override;
        // Handle mouse events recursively and bring the current window to the top
        bool mouse_button_event(const Vector2i& p, i32 button, bool down, i32 modifiers) override;
        // Accept scroll events and propagate them to the widget under the mouse cursor
        bool scroll_event(const Vector2i& p, const Vector2f& rel) override;
        // Compute the preferred size of the widget
        Vector2i preferred_size(SDL3::SDL_Renderer* ctx) const override;
        // Invoke the associated layout generator to properly place child widgets, if any
        void perform_layout(SDL3::SDL_Renderer* ctx) override;

        // Handle a focus change event.
        // default implementation: record the focus status, but do nothing
        bool focus_event(bool focused) override;

    protected:
        // Internal helper function to maintain nested window position values.
        // overridden in Popup
        void refresh_relative_placement();

    protected:
        struct AsyncTexture;
        using AsyncTexturePtr = std::shared_ptr<Window::AsyncTexture>;

        bool m_modal{ false };
        bool m_drag{ false };
        bool m_draggable{ true };
        bool m_drop_shadow_enabled{ true };

        std::string m_title{};
        Widget* m_button_panel{ nullptr };
        Texture m_title_texture{};

        std::vector<Window::AsyncTexturePtr> m_window_textures{};
        Window::AsyncTexturePtr m_curr_texture{ nullptr };

    private:
        void draw_texture(Window::AsyncTexturePtr& texture, SDL3::SDL_Renderer* renderer);
    };
}
