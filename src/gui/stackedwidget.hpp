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

        void set_selected_index(size_t index);
        size_t selected_idx() const;

        void perform_layout(SDL3::SDL_Renderer* ctx) override;
        Vector2i preferred_size(SDL3::SDL_Renderer* ctx) const override;
        void add_child(size_t index, Widget* widget) override;

    private:
        size_t m_selected_idx = std::numeric_limits<size_t>::max();
    };
}
