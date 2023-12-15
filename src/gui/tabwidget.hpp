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

        void setActiveTab(int tabIndex);
        int activeTab() const;
        int tabCount() const;

        /**
         * Sets the callable objects which is invoked when a tab is changed.
         * The argument provided to the callback is the index of the new active tab.
         */
        void setCallback(const std::function<void(int)>& callback)
        {
            mCallback = callback;
        };

        const std::function<void(int)>& callback() const
        {
            return mCallback;
        }

        /// Creates a new tab with the specified name and returns a pointer to the layer.
        Widget* createTab(const std::string& label);
        Widget* createTab(int index, const std::string& label);

        /// Inserts a tab at the end of the tabs collection and associates it with the provided
        /// widget.
        void addTab(const std::string& label, Widget* tab);

        /// Inserts a tab into the tabs collection at the specified index and associates it with the
        /// provided widget.
        void addTab(int index, const std::string& label, Widget* tab);

        /**
         * Removes the tab with the specified label and returns the index of the label.
         * Returns whether the removal was successful.
         */
        bool removeTab(const std::string& label);

        /// Removes the tab with the specified index.
        void removeTab(int index);

        /// Retrieves the label of the tab at a specific index.
        const std::string& tabLabelAt(int index) const;

        /**
         * Retrieves the index of a specific tab using its tab label.
         * Returns -1 if there is no such tab.
         */
        int tabLabelIndex(const std::string& label);

        /**
         * Retrieves the index of a specific tab using a widget pointer.
         * Returns -1 if there is no such tab.
         */
        int tabIndex(Widget* tab);

        /**
         * This function can be invoked to ensure that the tab with the provided
         * index the is visible, i.e to track the given tab. Forwards to the tab
         * header widget. This function should be used whenever the client wishes
         * to make the tab header follow a newly added tab, as the content of the
         * new tab is made visible but the tab header does not track it by default.
         */
        void ensureTabVisible(int index);

        const Widget* tab(const std::string& label) const;
        Widget* tab(const std::string& label);

        void performLayout(SDL3::SDL_Renderer* ctx) override;
        Vector2i preferredSize(SDL3::SDL_Renderer* ctx) const override;
        void draw(SDL3::SDL_Renderer* ctx) override;

    private:
        TabHeader* mHeader;
        StackedWidget* mContent;
        std::function<void(int)> mCallback;
    };
}