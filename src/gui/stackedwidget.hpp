#pragma once

#include "gui/widget.hpp"

namespace rl::gui {

    /**
     * \class StackedWidget stackedwidget.h sdl_gui/stackedwidget.h
     *
     * \brief A stack widget.
     */
    class StackedWidget : public Widget
    {
    public:
        StackedWidget(Widget* parent);

        void setSelectedIndex(int index);
        int selectedIndex() const;

        void performLayout(SDL3::SDL_Renderer* ctx) override;
        Vector2i preferredSize(SDL3::SDL_Renderer* ctx) const override;
        void addChild(int index, Widget* widget) override;

    private:
        int mSelectedIndex = -1;
    };
}
