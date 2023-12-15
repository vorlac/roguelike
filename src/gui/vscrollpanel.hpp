#pragma once

#include "gui/widget.hpp"

namespace rl::gui {
    /**
     * @brief Adds a vertical scrollbar around a widget that is too big to fit into
     *        a certain area.
     */
    class VScrollPanel : public Widget
    {
    public:
        VScrollPanel(Widget* parent);

        void perform_layout(SDL3::SDL_Renderer* ctx) override;
        Vector2i preferredSize(SDL3::SDL_Renderer* ctx) const override;
        bool mouseDragEvent(const Vector2i& p, const Vector2i& rel, int button,
                            int modifiers) override;
        bool scrollEvent(const Vector2i& p, const Vector2f& rel) override;
        bool mouseButtonEvent(const Vector2i& p, int button, bool down, int modifiers) override;
        bool mouseMotionEvent(const Vector2i& p, const Vector2i& rel, int button,
                              int modifiers) override;
        void draw(SDL3::SDL_Renderer* render) override;

        SDL3::SDL_Point get_absolute_pos() const override;
        PntRect get_absolute_cliprect() const override;
        int get_absolute_top() const override;

    protected:
        int mChildPreferredHeight;
        float mScroll;
        int mDOffset = 0;
    };
}
