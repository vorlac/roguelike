#include "core/assert.hpp"
#include "gui/stackedwidget.hpp"

namespace rl::gui {
    StackedWidget::StackedWidget(Widget* parent)
        : Widget(parent)
    {
    }

    void StackedWidget::setSelectedIndex(size_t index)
    {
        runtime_assert(index < this->child_count(), "child widget index out of bounds");
        if (m_selected_idx >= 0)
            m_children[m_selected_idx]->set_visible(false);
        m_selected_idx = index;
        m_children[m_selected_idx]->set_visible(true);
    }

    size_t StackedWidget::selected_idx() const
    {
        return m_selected_idx;
    }

    void StackedWidget::perform_layout(SDL3::SDL_Renderer* ctx)
    {
        for (auto child : m_children)
        {
            child->set_relative_position({ 0, 0 });
            child->set_size(m_size);
            child->perform_layout(ctx);
        }
    }

    Vector2i StackedWidget::preferredSize(SDL3::SDL_Renderer* ctx) const
    {
        Vector2i size{ 0, 0 };
        for (auto child : m_children)
            size = size.cmax(child->preferredSize(ctx));
        return size;
    }

    void StackedWidget::add_child(size_t index, Widget* widget)
    {
        if (m_selected_idx >= 0)
            m_children[m_selected_idx]->set_visible(false);
        Widget::add_child(index, widget);
        widget->set_visible(true);
        setSelectedIndex(index);
    }
}
