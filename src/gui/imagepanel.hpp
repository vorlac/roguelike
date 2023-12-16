#pragma once

#include "gui/widget.hpp"

namespace rl::gui {
    /**
     * \class ImagePanel imagepanel.h sdl_gui/imagepanel.h
     *
     * \brief Image panel widget which shows a number of square-shaped icons.
     */
    class ImagePanel : public Widget
    {
    public:
        ImagePanel(Widget* parent);

        ImagePanel(Widget* parent, const ListImages& data)
            : ImagePanel(parent)
        {
            setImages(data);
        }

        void setImages(const ListImages& data)
        {
            mImages = data;
        }

        const ListImages& images() const
        {
            return mImages;
        }

        std::function<void(int)> callback() const
        {
            return m_callback;
        }

        void set_callback(const std::function<void(int)>& callback)
        {
            m_callback = callback;
        }

        bool mouse_motion_event(const Vector2i& p, const Vector2i& rel, int button,
                                int modifiers) override;
        bool mouse_button_event(const Vector2i& p, int button, bool down, int modifiers) override;
        Vector2i preferred_size(SDL3::SDL_Renderer* ctx) const override;
        void draw(SDL3::SDL_Renderer* renderer) override;

        ImagePanel& withImages(const ListImages& data)
        {
            setImages(data);
            return *this;
        }

    protected:
        Vector2i gridSize() const;
        int indexForPosition(const Vector2i& p) const;

    protected:
        ListImages mImages;
        std::function<void(int)> m_callback;
        int mThumbSize;
        int mSpacing;
        int mMargin;
        int mMouseIndex;
    };
}
