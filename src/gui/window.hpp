#pragma once

#include <memory>

#include "gui/widget.hpp"

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
            set_relative_position(pos);
        }

        // Return the window title
        const std::string& title() const
        {
            return mTitle;
        }

        // Set the window title
        void setTitle(const std::string& title)
        {
            mTitle = title;
        }

        // Is this a model dialog?
        bool modal() const
        {
            return mModal;
        }

        // Set whether or not this is a modal dialog
        void setModal(bool modal)
        {
            mModal = modal;
        }

        // Is this draggable?
        bool draggable() const
        {
            return mDraggable;
        }

        // Set whether or not this is draggable
        void setDraggable(bool draggable)
        {
            mDraggable = draggable;
        }

        // Drop shadow enabled?
        bool dropShadowEnabled() const
        {
            return mDropShadowEnabled;
        }

        // Set whether or not drop shadow is enabled
        void setDropShadowEnabled(bool dropShadowEnabled)
        {
            mDropShadowEnabled = dropShadowEnabled;
        }

        // Return the panel used to house window buttons
        Widget* buttonPanel();

        // Dispose the window
        void dispose();

        // Center the window in the current \ref Screen
        void center();

        // Draw the window
        void draw(SDL3::SDL_Renderer* surface) override;
        virtual void drawBody(SDL3::SDL_Renderer* renderer);
        virtual void drawBodyTemp(SDL3::SDL_Renderer* renderer);

        // Handle window drag events
        bool mouseDragEvent(const Vector2i& p, const Vector2i& rel, int button,
                            int modifiers) override;
        // Handle mouse events recursively and bring the current window to the top
        bool mouseButtonEvent(const Vector2i& p, int button, bool down, int modifiers) override;
        // Accept scroll events and propagate them to the widget under the mouse cursor
        bool scrollEvent(const Vector2i& p, const Vector2f& rel) override;
        // Compute the preferred size of the widget
        Vector2i preferredSize(SDL3::SDL_Renderer* ctx) const override;
        // Invoke the associated layout generator to properly place child widgets, if any
        void perform_layout(SDL3::SDL_Renderer* ctx) override;

        // Handle a focus change event.
        // default implementation: record the focus status, but do nothing
        bool focusEvent(bool focused) override;

    protected:
        // Internal helper function to maintain nested window position values.
        // overridden in Popup
        void refreshRelativePlacement();

    protected:
        std::string mTitle;
        Widget* mButtonPanel;

        Texture _titleTex;

        bool mModal;
        bool mDrag;
        bool mDraggable = true;
        bool mDropShadowEnabled = true;

        struct AsyncTexture;
        typedef std::shared_ptr<AsyncTexture> AsyncTexturePtr;
        std::vector<Window::AsyncTexturePtr> m_window_txs;

        AsyncTexturePtr current_texture_ = nullptr;

    private:
        void drawTexture(Window::AsyncTexturePtr& texture, SDL3::SDL_Renderer* renderer);
    };
}
