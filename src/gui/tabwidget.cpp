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
        , mHeader(new TabHeader(this))
        , mContent(new StackedWidget(this))
    {
        mHeader->setCallback([this](size_t idx) {
            mContent->setSelectedIndex(idx);
            if (m_active_tab_changed_callback)
                m_active_tab_changed_callback(idx);
        });
    }

    void TabWidget::setActiveTab(size_t idx)
    {
        mHeader->setActiveTab(idx);
        mContent->setSelectedIndex(idx);
    }

    size_t TabWidget::activeTab() const
    {
        runtime_assert(
            mHeader->activeTab() == mContent->selected_idx(),
            "TabWidget: selected header tab index inconsistent with selected content header: h:{} c:{}",
            mHeader->activeTab(), mContent->selected_idx());
        return mContent->selected_idx();
    }

    size_t TabWidget::tabCount() const
    {
        runtime_assert(
            mContent->child_count() == mHeader->tabCount(),
            "TabWidget: content child count inconsistent with header tab count\n h:{} c:{}",
            mContent->child_count(), mHeader->tabCount());
        return mHeader->tabCount();
    }

    Widget* TabWidget::createTab(size_t index, const std::string& label)
    {
        Widget* tab = new Widget(nullptr);
        this->addTab(index, label, tab);
        return tab;
    }

    Widget* TabWidget::createTab(const std::string& label)
    {
        return this->createTab(this->tabCount(), label);
    }

    void TabWidget::addTab(const std::string& name, Widget* tab)
    {
        this->addTab(this->tabCount(), name, tab);
    }

    void TabWidget::addTab(size_t index, const std::string& label, Widget* tab)
    {
        runtime_assert(index <= this->tabCount(),
                       "TabWidget: tab index out of bounds (index:{}, count:{})", index,
                       this->tabCount());
        // It is important to add the content first since the callback
        // of the header will automatically fire when a new tab is added.
        mContent->add_child(index, tab);
        mHeader->addTab(index, label);

        runtime_assert(
            mHeader->tabCount() == mContent->child_count(),
            "TabWidget: header tab count inconsistent with content child count\n h:{} c:{}",
            mHeader->tabCount(), mContent->child_count());
    }

    size_t TabWidget::tabLabelIndex(const std::string& label)
    {
        return mHeader->tabIndex(label);
    }

    size_t TabWidget::tabIndex(Widget* tab)
    {
        return mContent->get_child_index(tab);
    }

    void TabWidget::ensureTabVisible(size_t index)
    {
        if (!mHeader->isTabVisible(index))
            mHeader->ensureTabVisible(index);
    }

    const Widget* TabWidget::tab(const std::string& tabName) const
    {
        int index = mHeader->tabIndex(tabName);
        if (index == mContent->child_count())
            return nullptr;
        return mContent->children()[index];
    }

    Widget* TabWidget::tab(const std::string& tabName)
    {
        int index = mHeader->tabIndex(tabName);
        if (index == mContent->child_count())
            return nullptr;
        return mContent->children()[index];
    }

    bool TabWidget::removeTab(const std::string& tabName)
    {
        int index = mHeader->removeTab(tabName);
        if (index == -1)
            return false;
        mContent->remove_child(index);
        return true;
    }

    void TabWidget::removeTab(size_t index)
    {
        assert(mContent->child_count() < index);
        mHeader->removeTab(index);
        mContent->remove_child(index);

        if (this->activeTab() == index)
            this->setActiveTab(index == (index - 1) ? index - 1 : 0);
    }

    const std::string& TabWidget::tabLabelAt(size_t index) const
    {
        return mHeader->tabLabelAt(index);
    }

    void TabWidget::perform_layout(SDL3::SDL_Renderer* ctx)
    {
        int headerHeight = mHeader->preferredSize(ctx).y;
        int margin = m_theme->mTabInnerMargin;
        mHeader->set_relative_position({ 0, 0 });
        mHeader->set_size({ m_size.x, headerHeight });
        mHeader->perform_layout(ctx);
        mContent->set_relative_position({ margin, headerHeight + margin });
        mContent->set_size({ m_size.x - 2 * margin, m_size.y - 2 * margin - headerHeight });
        mContent->perform_layout(ctx);
    }

    Vector2i TabWidget::preferredSize(SDL3::SDL_Renderer* ctx) const
    {
        auto contentSize = mContent->preferredSize(ctx);
        auto headerSize = mHeader->preferredSize(ctx);
        int margin = m_theme->mTabInnerMargin;
        auto borderSize = Vector2i{ 2 * margin, 2 * margin };
        Vector2i tabPreferredSize = contentSize + borderSize + Vector2i{ 0, headerSize.y };
        return tabPreferredSize;
    }

    void TabWidget::draw(SDL3::SDL_Renderer* renderer)
    {
        int tabHeight = mHeader->preferredSize(nullptr).y;
        auto activeArea = mHeader->activeButtonArea();

        for (int i = 0; i < 3; ++i)
        {
            int x = get_absolute_left();
            int y = get_absolute_top();
            SDL3::SDL_Color bl = m_theme->mBorderLight.toSdlColor();
            SDL3::SDL_Rect blr{ x + 1, y + tabHeight + 2, m_size.x - 2, m_size.y - tabHeight - 2 };

            SDL_SetRenderDrawColor(renderer, bl.r, bl.g, bl.b, bl.a);
            SDL_RenderLine(renderer, blr.x, blr.y, x + activeArea.first.x, blr.y);
            SDL_RenderLine(renderer, x + activeArea.second.x, blr.y, blr.x + blr.w, blr.y);
            SDL_RenderLine(renderer, blr.x + blr.w, blr.y, blr.x + blr.w, blr.y + blr.h);
            SDL_RenderLine(renderer, blr.x, blr.y, blr.x, blr.y + blr.h);
            SDL_RenderLine(renderer, blr.x, blr.y + blr.h, blr.x + blr.w, blr.y + blr.h);

            SDL3::SDL_Color bd = m_theme->mBorderDark.toSdlColor();
            SDL3::SDL_Rect bdr{ x + 1, y + tabHeight + 1, m_size.x - 2, m_size.y - tabHeight - 2 };

            SDL_SetRenderDrawColor(renderer, bd.r, bd.g, bd.b, bd.a);
            SDL_RenderLine(renderer, bdr.x, bdr.y, x + activeArea.first.x, bdr.y);
            SDL_RenderLine(renderer, x + activeArea.second.x, bdr.y, bdr.x + bdr.w, bdr.y);
            SDL_RenderLine(renderer, bdr.x + bdr.w, bdr.y, bdr.x + bdr.w, bdr.y + bdr.h);
            SDL_RenderLine(renderer, bdr.x, bdr.y, bdr.x, bdr.y + bdr.h);
            SDL_RenderLine(renderer, bdr.x, bdr.y + bdr.h, bdr.x + bdr.w, bdr.y + bdr.h);
        }

        Widget::draw(renderer);
    }
}
