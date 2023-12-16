#include <algorithm>

#include "core/assert.hpp"
#include "gui/screen.hpp"
#include "gui/stackedwidget.hpp"
#include "gui/tabheader.hpp"
#include "gui/tabwidget.hpp"
#include "gui/theme.hpp"
#include "gui/window.hpp"

namespace rl::gui {
    TabWidget::TabWidget(Widget* parent)
        : Widget(parent)
        , m_tab_header(new TabHeader(this))
        , m_content(new StackedWidget(this))
    {
        m_tab_header->set_callback([this](size_t idx) {
            m_content->set_selected_index(idx);
            if (m_active_tab_changed_callback)
                m_active_tab_changed_callback(idx);
        });
    }

    void TabWidget::set_active_tab(size_t idx)
    {
        m_tab_header->set_active_tab(idx);
        m_content->set_selected_index(idx);
    }

    size_t TabWidget::active_tab() const
    {
        runtime_assert(
            m_tab_header->active_tab() == m_content->selected_idx(),
            "TabWidget: selected header tab index inconsistent with selected content header: h:{} c:{}",
            m_tab_header->active_tab(), m_content->selected_idx());
        return m_content->selected_idx();
    }

    size_t TabWidget::tab_count() const
    {
        runtime_assert(
            m_content->child_count() == m_tab_header->tab_count(),
            "TabWidget: content child count inconsistent with header tab count\n h:{} c:{}",
            m_content->child_count(), m_tab_header->tab_count());
        return m_tab_header->tab_count();
    }

    Widget* TabWidget::create_tab(size_t index, const std::string& label)
    {
        Widget* tab = new Widget(nullptr);
        this->add_tab(index, label, tab);
        return tab;
    }

    Widget* TabWidget::create_tab(const std::string& label)
    {
        return this->create_tab(this->tab_count(), label);
    }

    void TabWidget::add_tab(const std::string& name, Widget* tab)
    {
        this->add_tab(this->tab_count(), name, tab);
    }

    void TabWidget::add_tab(size_t index, const std::string& label, Widget* tab)
    {
        runtime_assert(index <= this->tab_count(),
                       "TabWidget: tab index out of bounds (index:{}, count:{})", index,
                       this->tab_count());
        // It is important to add the content first since the callback
        // of the header will automatically fire when a new tab is added.
        m_content->add_child(index, tab);
        m_tab_header->add_tab(index, label);

        runtime_assert(
            m_tab_header->tab_count() == m_content->child_count(),
            "TabWidget: header tab count inconsistent with content child count\n h:{} c:{}",
            m_tab_header->tab_count(), m_content->child_count());
    }

    size_t TabWidget::tab_label_index(const std::string& label)
    {
        return m_tab_header->tab_index(label);
    }

    size_t TabWidget::tab_index(Widget* tab)
    {
        return m_content->get_child_index(tab);
    }

    void TabWidget::ensure_tab_visible(size_t index)
    {
        if (!m_tab_header->isTabVisible(index))
            m_tab_header->ensure_tab_visible(index);
    }

    const Widget* TabWidget::tab(const std::string& tabName) const
    {
        int index = m_tab_header->tab_index(tabName);
        if (index == m_content->child_count())
            return nullptr;
        return m_content->children()[index];
    }

    Widget* TabWidget::tab(const std::string& tabName)
    {
        int index = m_tab_header->tab_index(tabName);
        if (index == m_content->child_count())
            return nullptr;
        return m_content->children()[index];
    }

    bool TabWidget::remove_tab(const std::string& tabName)
    {
        int index = m_tab_header->remove_tab(tabName);
        if (index == -1)
            return false;
        m_content->remove_child(index);
        return true;
    }

    void TabWidget::remove_tab(size_t index)
    {
        runtime_assert(m_content->child_count() < index, "child index out of bounds");
        m_tab_header->remove_tab(index);
        m_content->remove_child(index);

        if (this->active_tab() == index)
            this->set_active_tab(index == (index - 1) ? index - 1 : 0);
    }

    const std::string& TabWidget::tab_label_at(size_t index) const
    {
        return m_tab_header->tab_label_at(index);
    }

    void TabWidget::perform_layout(SDL3::SDL_Renderer* ctx)
    {
        int headerHeight = m_tab_header->preferred_size(ctx).y;
        int margin = m_theme->m_tab_inner_margin;
        m_tab_header->set_relative_position({ 0, 0 });
        m_tab_header->set_size({ m_size.x, headerHeight });
        m_tab_header->perform_layout(ctx);
        m_content->set_relative_position({ margin, headerHeight + margin });
        m_content->set_size({ m_size.x - 2 * margin, m_size.y - 2 * margin - headerHeight });
        m_content->perform_layout(ctx);
    }

    Vector2i TabWidget::preferred_size(SDL3::SDL_Renderer* ctx) const
    {
        auto contentSize = m_content->preferred_size(ctx);
        auto headerSize = m_tab_header->preferred_size(ctx);
        int margin = m_theme->m_tab_inner_margin;
        auto borderSize = Vector2i{ 2 * margin, 2 * margin };
        Vector2i tabPreferredSize = contentSize + borderSize + Vector2i{ 0, headerSize.y };
        return tabPreferredSize;
    }

    void TabWidget::draw(SDL3::SDL_Renderer* renderer)
    {
        int tabHeight = m_tab_header->preferred_size(nullptr).y;
        auto activeArea = m_tab_header->active_button_area();

        for (int i = 0; i < 3; ++i)
        {
            int x = get_absolute_left();
            int y = get_absolute_top();
            SDL3::SDL_Color bl = m_theme->m_border_light.sdl_color();
            SDL3::SDL_Rect blr{ x + 1, y + tabHeight + 2, m_size.x - 2, m_size.y - tabHeight - 2 };

            SDL3::SDL_SetRenderDrawColor(renderer, bl.r, bl.g, bl.b, bl.a);
            SDL3::SDL_RenderLine(renderer, blr.x, blr.y, x + activeArea.first.x, blr.y);
            SDL3::SDL_RenderLine(renderer, x + activeArea.second.x, blr.y, blr.x + blr.w, blr.y);
            SDL3::SDL_RenderLine(renderer, blr.x + blr.w, blr.y, blr.x + blr.w, blr.y + blr.h);
            SDL3::SDL_RenderLine(renderer, blr.x, blr.y, blr.x, blr.y + blr.h);
            SDL3::SDL_RenderLine(renderer, blr.x, blr.y + blr.h, blr.x + blr.w, blr.y + blr.h);

            SDL3::SDL_Color bd = m_theme->m_border_dark.sdl_color();
            SDL3::SDL_Rect bdr{ x + 1, y + tabHeight + 1, m_size.x - 2, m_size.y - tabHeight - 2 };

            SDL3::SDL_SetRenderDrawColor(renderer, bd.r, bd.g, bd.b, bd.a);
            SDL3::SDL_RenderLine(renderer, bdr.x, bdr.y, x + activeArea.first.x, bdr.y);
            SDL3::SDL_RenderLine(renderer, x + activeArea.second.x, bdr.y, bdr.x + bdr.w, bdr.y);
            SDL3::SDL_RenderLine(renderer, bdr.x + bdr.w, bdr.y, bdr.x + bdr.w, bdr.y + bdr.h);
            SDL3::SDL_RenderLine(renderer, bdr.x, bdr.y, bdr.x, bdr.y + bdr.h);
            SDL3::SDL_RenderLine(renderer, bdr.x, bdr.y + bdr.h, bdr.x + bdr.w, bdr.y + bdr.h);
        }

        Widget::draw(renderer);
    }
}
