/*
    sdlgui/combobox.cpp -- simple combo box widget based on a popup button

    Based on NanoGUI by Wenzel Jakob <wenzel@inf.ethz.ch>.
    Adaptation for SDL by Dalerank <dalerankn8@gmail.com>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <cassert>

#include "gui/combobox.hpp"
#include "gui/layout.hpp"

namespace rl::gui {
    ComboBox::ComboBox(Widget* parent)
        : PopupButton(parent)
        , mSelectedIndex(0)
    {
    }

    ComboBox::ComboBox(Widget* parent, const std::vector<std::string>& items)
        : PopupButton(parent)
        , mSelectedIndex(0)
    {
        setItems(items);
    }

    ComboBox::ComboBox(Widget* parent, const std::vector<std::string>& items,
                       const std::vector<std::string>& itemsShort)
        : PopupButton(parent)
        , mSelectedIndex(0)
    {
        setItems(items, itemsShort);
    }

    void ComboBox::set_selected_index(int idx)
    {
        if (mItemsShort.empty())
            return;

        const std::vector<Widget*>& children = popup().children();
        ((Button*)children[mSelectedIndex])->set_pushed(false);
        ((Button*)children[idx])->set_pushed(true);
        mSelectedIndex = idx;
        set_caption(mItemsShort[idx]);
    }

    void ComboBox::setItems(const std::vector<std::string>& items,
                            const std::vector<std::string>& itemsShort)
    {
        assert(items.size() == itemsShort.size());
        mItems = items;
        mItemsShort = itemsShort;
        if (mSelectedIndex < 0 || mSelectedIndex >= (int)items.size())
            mSelectedIndex = 0;
        while (mPopup->child_count() != 0)
            mPopup->remove_child(mPopup->child_count() - 1);
        mPopup->set_layout(new GroupLayout(10));
        int index = 0;
        for (const auto& str : items)
        {
            Button* button = new Button(mPopup, str);
            button->set_flags(Button::RadioButton);
            button->set_callback([&, index] {
                mSelectedIndex = index;
                set_caption(mItemsShort[index]);
                set_pushed(false);
                popup().set_visible(false);

                if (m_cb_pressed_callback)
                    m_cb_pressed_callback(index);
            });
            index++;
        }
        set_selected_index(mSelectedIndex);
    }

    bool ComboBox::scroll_event(const Vector2i& p, const Vector2f& rel)
    {
        if (rel.y < 0)
        {
            set_selected_index(std::min(mSelectedIndex + 1, (int)(items().size() - 1)));
            if (m_cb_pressed_callback)
                m_cb_pressed_callback(mSelectedIndex);
            return true;
        }
        else if (rel.y > 0)
        {
            set_selected_index(std::max(mSelectedIndex - 1, 0));
            if (m_cb_pressed_callback)
                m_cb_pressed_callback(mSelectedIndex);
            return true;
        }
        return Widget::scroll_event(p, rel);
    }
}
