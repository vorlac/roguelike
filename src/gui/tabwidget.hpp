#pragma once

#include <functional>

#include "gui/widget.hpp"

namespace rl::gui {
    class StackedWidget;
    class TabHeader;

    /**
     * \class TabWidget tabwidget.h sdl_gui/tabwidget.h
     *
     * \brief A wrapper around the widgets TabHeader and StackedWidget which hooks
     *        the two classes together.
     */
    class TabWidget : public Widget
    {
    public:
        TabWidget(Widget* parent);

        void setActiveTab(size_t idx);
        size_t activeTab() const;
        size_t tabCount() const;

        /**
         * Sets the callable objects which is invoked when a tab is changed.
         * The argument provided to the callback is the index of the new active tab.
         */
        void setCallback(std::function<void(size_t)>&& callback)
        {
            m_active_tab_changed_callback = std::move(callback);
        };

        const std::function<void(size_t)>& callback() const
        {
            return m_active_tab_changed_callback;
        }

        // Creates a new tab with the specified name and returns a pointer to the layer.
        Widget* createTab(const std::string& label);
        Widget* createTab(size_t index, const std::string& label);

        // Inserts a tab at the end of the tabs collection and associates it with the provided
        // widget.
        void addTab(const std::string& label, Widget* tab);

        // Inserts a tab into the tabs collection at the specified index and associates it with the
        // provided widget.
        void addTab(size_t index, const std::string& label, Widget* tab);

        /**
         * Removes the tab with the specified label and returns the index of the label.
         * Returns whether the removal was successful.
         */
        bool removeTab(const std::string& label);

        // Removes the tab with the specified index.
        void removeTab(size_t index);

        // Retrieves the label of the tab at a specific index.
        const std::string& tabLabelAt(size_t index) const;

        /**
         * Retrieves the index of a specific tab using its tab label.
         * Returns -1 if there is no such tab.
         */
        size_t tabLabelIndex(const std::string& label);

        /**
         * Retrieves the index of a specific tab using a widget pointer.
         * Returns -1 if there is no such tab.
         */
        size_t tabIndex(Widget* tab);

        /**
         * This function can be invoked to ensure that the tab with the provided
         * index the is visible, i.e to track the given tab. Forwards to the tab
         * header widget. This function should be used whenever the client wishes
         * to make the tab header follow a newly added tab, as the content of the
         * new tab is made visible but the tab header does not track it by default.
         */
        void ensureTabVisible(size_t index);

        const Widget* tab(const std::string& label) const;
        Widget* tab(const std::string& label);

        void perform_layout(SDL3::SDL_Renderer* ctx) override;
        Vector2i preferredSize(SDL3::SDL_Renderer* ctx) const override;
        void draw(SDL3::SDL_Renderer* ctx) override;

    private:
        TabHeader* mHeader;
        StackedWidget* mContent;
        std::function<void(size_t)> m_active_tab_changed_callback;
    };
}
