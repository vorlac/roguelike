#pragma once

#include "gui/widget.hpp"

namespace rl::gui {

    /**
     * \class VScrollPanel vscrollpanel.h nanogui/vscrollpanel.h
     *
     * \brief Adds a vertical scrollbar around a widget that is too big to fit into
     *        a certain area.
     */
    class VScrollPanel : public Widget
    {
    public:
        VScrollPanel(Widget* parent);

        /**
         * Return the current scroll amount as a value between 0 and 1. 0 means
         * scrolled to the top and 1 to the bottom.
         */
        float scroll() const
        {
            return m_scroll;
        }

        /**
         * Set the scroll amount to a value between 0 and 1. 0 means scrolled to
         * the top and 1 to the bottom.
         */
        void set_scroll(float scroll)
        {
            m_scroll = scroll;
        }

        virtual void perform_layout(NVGcontext* ctx) override;
        virtual Vector2i preferred_size(NVGcontext* ctx) const override;
        virtual bool mouse_button_event(const Vector2i& p, int button, bool down,
                                        int modifiers) override;
        virtual bool mouse_drag_event(const Vector2i& p, const Vector2i& rel, int button,
                                      int modifiers) override;
        virtual bool scroll_event(const Vector2i& p, const Vector2f& rel) override;
        virtual void draw(NVGcontext* ctx) override;

    protected:
        int m_child_preferred_height;
        float m_scroll;
        bool m_update_layout;
    };

}