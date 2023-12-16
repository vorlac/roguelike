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

        void set_active_tab(size_t idx);
        size_t active_tab() const;
        size_t tab_count() const;

        /**
         * Sets the callable objects which is invoked when a tab is changed.
         * The argument provided to the callback is the index of the new active tab.
         */
        void set_callback(std::function<void(size_t)>&& callback)
        {
            m_active_tab_changed_callback = std::move(callback);
        };

        const std::function<void(size_t)>& callback() const
        {
            return m_active_tab_changed_callback;
        }

        // Creates a new tab with the specified name and returns a pointer to the layer.
        Widget* create_tab(const std::string& label);
        Widget* create_tab(size_t index, const std::string& label);

        // Inserts a tab at the end of the tabs collection and associates it with the provided
        // widget.
        void add_tab(const std::string& label, Widget* tab);

        // Inserts a tab into the tabs collection at the specified index and associates it with the
        // provided widget.
        void add_tab(size_t index, const std::string& label, Widget* tab);

        /**
         * Removes the tab with the specified label and returns the index of the label.
         * Returns whether the removal was successful.
         */
        bool remove_tab(const std::string& label);

        // Removes the tab with the specified index.
        void remove_tab(size_t index);

        // Retrieves the label of the tab at a specific index.
        const std::string& tab_label_at(size_t index) const;

        /**
         * Retrieves the index of a specific tab using its tab label.
         * Returns -1 if there is no such tab.
         */
        size_t tab_label_index(const std::string& label);

        /**
         * Retrieves the index of a specific tab using a widget pointer.
         * Returns -1 if there is no such tab.
         */
        size_t tab_index(Widget* tab);

        /**
         * This function can be invoked to ensure that the tab with the provided
         * index the is visible, i.e to track the given tab. Forwards to the tab
         * header widget. This function should be used whenever the client wishes
         * to make the tab header follow a newly added tab, as the content of the
         * new tab is made visible but the tab header does not track it by default.
         */
        void ensure_tab_visible(size_t index);

        const Widget* tab(const std::string& label) const;
        Widget* tab(const std::string& label);

        void perform_layout(SDL3::SDL_Renderer* ctx) override;
        Vector2i preferred_size(SDL3::SDL_Renderer* ctx) const override;
        void draw(SDL3::SDL_Renderer* ctx) override;

    private:
        TabHeader* m_tab_header;
        StackedWidget* m_content;
        std::function<void(size_t)> m_active_tab_changed_callback;
    };
}
