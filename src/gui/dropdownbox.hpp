#pragma once

#include <functional>
#include <string>
#include <vector>

#include "gui/common.hpp"
#include "gui/popupbutton.hpp"

namespace rl::gui {
    /**
     * \class DropdownBox dropdownbox.h nanogui/dropdownbox.h
     *
     * \brief Simple dropdownbox box widget based on a popup button.
     */
    class DropdownBox : public PopupButton
    {
    public:
        /// Create an empty combo box
        DropdownBox(Widget* parent);

        /// Create a new combo box with the given items
        DropdownBox(Widget* parent, const std::vector<std::string>& items);

        /**
         * \brief Create a new dropdownbox with the given items, providing both short and
         * long descriptive labels for each item
         */
        DropdownBox(Widget* parent, const std::vector<std::string>& items,
                    const std::vector<std::string>& itemsShort);

        /// The callback to execute for this widget.
        const std::function<void(int)>& callback() const
        {
            return m_popup_callback;
        }

        /// Sets the callback to execute for this widget.
        void set_callback(std::function<void(int)>&& callback)
        {
            m_popup_callback = std::move(callback);
        }

        /// The current index this dropdownbox has selected.
        int selected_idx() const
        {
            return mSelectedIndex;
        }

        void perform_layout(SDL3::SDL_Renderer* renderer) override;

        /// Sets the current index this dropdownbox has selected.
        void set_selected_index(int idx);

        /// Sets the items for this dropdownbox, providing both short and long descriptive lables
        /// for each item.
        void setItems(const std::vector<std::string>& items,
                      const std::vector<std::string>& itemsShort);

        /// Sets the items for this dropdownbox.
        void setItems(const std::vector<std::string>& items)
        {
            setItems(items, items);
        }

        /// The items associated with this dropdownbox.
        const std::vector<std::string>& items() const
        {
            return mItems;
        }

        /// The short descriptions associated with this dropdownbox.
        const std::vector<std::string>& itemsShort() const
        {
            return mItemsShort;
        }

        /// Handles mouse scrolling events for this dropdownbox.
        virtual bool scroll_event(const Vector2i& p, const Vector2f& rel) override;

        virtual void draw(SDL3::SDL_Renderer* renderer) override;
        virtual bool mouse_button_event(const Vector2i& p, int button, bool down,
                                        int modifiers) override;

    protected:
        /// The items associated with this dropdownbox.
        std::vector<std::string> mItems;

        /// The short descriptions of items associated with this dropdownbox.
        std::vector<std::string> mItemsShort;

        /// The callback for this dropdownbox.
        std::function<void(int)> m_popup_callback;

        /// The current index this dropdownbox has selected.
        int mSelectedIndex;
    };
}
