#pragma once

#include "gui/widget.hpp"

namespace rl::gui {

    /**
     * \class ImagePanel imagepanel.h nanogui/imagepanel.h
     *
     * \brief Image panel widget which shows a number of square-shaped icons.
     */
    class ImagePanel : public Widget
    {
    public:
        typedef std::vector<std::pair<int, std::string>> Images;

    public:
        ImagePanel(Widget* parent);

        void set_images(const Images& data)
        {
            m_images = data;
        }

        const Images& images() const
        {
            return m_images;
        }

        const std::function<void(int)>& callback() const
        {
            return m_callback;
        }

        void set_callback(const std::function<void(int)>& callback)
        {
            m_callback = callback;
        }

        virtual bool mouse_motion_event(const Vector2i& p, const Vector2i& rel, int button,
                                        int modifiers) override;
        virtual bool mouse_button_event(const Vector2i& p, int button, bool down,
                                        int modifiers) override;
        virtual Vector2i preferred_size(NVGcontext* ctx) const override;
        virtual void draw(NVGcontext* ctx) override;

    protected:
        Vector2i grid_size() const;
        int index_for_position(const Vector2i& p) const;

    protected:
        Images m_images;
        std::function<void(int)> m_callback;
        int m_thumb_size;
        int m_spacing;
        int m_margin;
        int m_mouse_index;
    };
}
