#pragma once

#include <memory>

#include "gui/popupbutton.hpp"

namespace rl::gui {
    class ComboBox : public PopupButton
    {
    public:
        /// Create an empty combo box
        ComboBox(Widget* parent);

        /// Create a new combo box with the given items
        // ComboBox(Widget *parent, const std::vector<std::string>& items={});
        ComboBox(Widget* parent, const std::vector<std::string>& items);

        /**
         * \brief Create a new combo box with the given items, providing both short and
         * long descriptive labels for each item
         */
        ComboBox(Widget* parent, const std::vector<std::string>& items,
                 const std::vector<std::string>& itemsShort);

        const std::function<void(int)>& callback() const
        {
            return m_cb_pressed_callback;
        }

        void set_callback(std::function<void(int)>&& callback)
        {
            m_cb_pressed_callback = std::move(callback);
        }

        int selected_idx() const
        {
            return mSelectedIndex;
        }

        void set_selected_index(int idx);

        void setItems(const std::vector<std::string>& items,
                      const std::vector<std::string>& itemsShort);

        void setItems(const std::vector<std::string>& items)
        {
            setItems(items, items);
        }

        const std::vector<std::string>& items() const
        {
            return mItems;
        }

        const std::vector<std::string>& itemsShort() const
        {
            return mItemsShort;
        }

        ComboBox& withItems(const std::vector<std::string>& items)
        {
            setItems(items);
            return *this;
        }

        bool scroll_event(const Vector2i& p, const Vector2f& rel) override;

    protected:
        std::vector<std::string> mItems, mItemsShort;
        std::function<void(int)> m_cb_pressed_callback;
        int mSelectedIndex;
    };
}
