#include <memory>
#include <string>
#include <vector>

#include "core/keyboard.hpp"
#include "core/mouse.hpp"
#include "core/ui/layouts/box_layout.hpp"
#include "core/ui/layouts/group_layout.hpp"
#include "core/ui/layouts/layout.hpp"
#include "core/ui/widget.hpp"
#include "core/ui/widgets/combobox.hpp"
#include "core/ui/widgets/vertical_scroll_panel.hpp"

namespace rl::ui {
    ComboBox::ComboBox(Widget* parent)
        : PopupButton{ parent }
        , m_item_container{ this->popup() }
    {
    }

    ComboBox::ComboBox(Widget* parent, const std::vector<std::string>& items)
        : PopupButton{ parent }
        , m_item_container{ this->popup() }
    {
        this->set_items(items);
    }

    ComboBox::ComboBox(Widget* parent, const std::vector<std::string>& items,
                       const std::vector<std::string>& items_short)
        : PopupButton{ parent }
        , m_item_container{ this->popup() }
    {
        using items_t = decltype(items);
        this->set_items(std::move(items), std::move(items_short));
    }

    i32 ComboBox::selected_index() const
    {
        return m_selected_index;
    }

    void ComboBox::set_selected_index(const i32 idx)
    {
        if (m_items_short.empty())
            return;

        const std::vector<Widget*>& children{ m_item_container->children() };
        dynamic_cast<Button*>(children[m_selected_index])->set_pressed(false);
        dynamic_cast<Button*>(children[static_cast<u32>(idx)])->set_pressed(true);
        m_selected_index = idx;

        this->set_text(m_items_short[static_cast<u32>(idx)]);
    }

    const std::function<void(u32)>& ComboBox::callback() const
    {
        return m_callback;
    }

    void ComboBox::set_callback(const std::function<void(u32)>& callback)
    {
        m_callback = callback;
    }

    const std::vector<std::string>& ComboBox::items() const
    {
        return m_items;
    }

    const std::vector<std::string>& ComboBox::items_short() const
    {
        return m_items_short;
    }

    i32 ComboBox::item_count() const
    {
        runtime_assert(m_items.size() == m_items_short.size(),
                       "Combo box: item counts mismatch: \n\titems:{} vs items_short:{}",
                       m_items.size(), m_items_short.size());

        return static_cast<i32>(m_items.size());
    }

    void ComboBox::set_items(const std::vector<std::string>& items)
    {
        this->set_items(items, items);
    }

    void ComboBox::set_items(const std::vector<std::string>& items,
                             const std::vector<std::string>& items_short)
    {
        runtime_assert(items.size() == items_short.size(), "item counts mismatch: {} vs {}",
                       items.size(), items_short.size());

        m_items = items;
        m_items_short = items_short;

        if (m_selected_index < 0 || m_selected_index >= this->item_count())
            m_selected_index = 0;

        while (m_item_container->child_count() != 0)
            m_item_container->remove_child_at(m_item_container->child_count() - 1);

        if (m_vscroll_panel == nullptr && items.size() > 8)
        {
            m_vscroll_panel = new VerticalScrollPanel{ m_popup };
            m_vscroll_panel->set_fixed_height(300);
            m_item_container = new Widget{ m_vscroll_panel };
            // m_popup->set_layout(new BoxLayout{
            //     Orientation::Horizontal,
            //     Alignment::Center,
            // });
        }

        // m_item_container->set_layout(new GroupLayout{ 5.0f });

        u32 index{ 0 };
        for (const auto& str : items)
        {
            auto button{ new Button{ m_item_container, str } };
            button->set_property(Button::Property::Radio);
            button->set_callback([this, index] {
                m_selected_index = static_cast<i32>(index);

                this->set_text(m_items_short[index]);
                this->set_pressed(false);
                this->popup()->set_visible(false);

                if (m_callback != nullptr)
                    m_callback(index);
            });

            index++;
        }

        this->set_selected_index(m_selected_index);
    }

    bool ComboBox::on_mouse_scroll(const Mouse& mouse, const Keyboard& kb)
    {
        this->set_pressed(false);
        this->popup()->set_visible(false);

        const auto mouse_wheel{ mouse.wheel_delta() };
        if (mouse_wheel.y < 0)
        {
            this->set_selected_index(
                std::min(m_selected_index + 1, static_cast<i32>(this->items().size() - 1)));

            if (m_callback != nullptr)
                m_callback(m_selected_index);

            return true;
        }

        if (mouse_wheel.y > 0)
        {
            this->set_selected_index(std::max(m_selected_index - 1, 0));
            if (m_callback != nullptr)
                m_callback(m_selected_index);

            return true;
        }

        return Widget::on_mouse_scroll(mouse, kb);
    }
}
