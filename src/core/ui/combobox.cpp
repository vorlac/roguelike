#include <vector>

#include "core/keyboard.hpp"
#include "core/mouse.hpp"
#include "core/ui/combobox.hpp"
#include "core/ui/layout.hpp"
#include "core/ui/vscrollpanel.hpp"
#include "core/ui/widget.hpp"

namespace rl::ui {

    ComboBox::ComboBox(ui::Widget* parent)
        : ui::PopupButton{ parent }
        , m_container{ this->popup() }
    {
    }

    ComboBox::ComboBox(ui::Widget* parent, const std::vector<std::string>& items)
        : ui::PopupButton{ parent }
        , m_container{ this->popup() }
    {
        this->set_items(items);
    }

    ComboBox::ComboBox(ui::Widget* parent, const std::vector<std::string>& items,
                       const std::vector<std::string>& items_short)
        : ui::PopupButton{ parent }
        , m_container{ this->popup() }
    {
        this->set_items(items, items_short);
    }

    i32 ComboBox::selected_index() const
    {
        return m_selected_index;
    }

    void ComboBox::set_selected_index(i32 idx)
    {
        if (m_items_short.empty())
            return;

        const std::vector<ui::Widget*>& children{ m_container->children() };
        static_cast<ui::Button*>(children[m_selected_index])->set_pressed(false);
        static_cast<ui::Button*>(children[idx])->set_pressed(true);

        m_selected_index = idx;

        this->set_caption(m_items_short[idx]);
    }

    const std::function<void(int)>& ComboBox::callback() const
    {
        return m_callback;
    }

    void ComboBox::set_callback(const std::function<void(i32)>& callback)
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

        if (m_selected_index < 0 || m_selected_index >= static_cast<int>(items.size()))
            m_selected_index = 0;

        while (m_container->child_count() != 0)
            m_container->remove_child_at(m_container->child_count() - 1);

        if (m_vscroll_panel == nullptr && items.size() > 8)
        {
            m_vscroll_panel = new ui::VScrollPanel{ m_popup };
            m_vscroll_panel->set_fixed_height(300);
            m_container = new ui::Widget{ m_vscroll_panel };
            m_popup->set_layout(new ui::BoxLayout{
                ui::Orientation::Horizontal,
                ui::Alignment::Center,
            });
        }

        m_container->set_layout(new ui::GroupLayout{ 10 });

        i32 index{ 0 };
        for (const auto& str : items)
        {
            ui::Button* button{ new ui::Button{ m_container, str } };
            button->set_flags(ui::Button::Flags::RadioButton);
            button->set_callback([&, index] {
                m_selected_index = index;

                this->set_caption(m_items_short[index]);
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

        auto&& mouse_wheel{ mouse.wheel_delta() };
        if (mouse_wheel.y < 0)
        {
            this->set_selected_index(
                std::min(m_selected_index + 1, static_cast<i32>(this->items().size() - 1)));

            if (m_callback != nullptr)
                m_callback(m_selected_index);

            return true;
        }
        else if (mouse_wheel.y > 0)
        {
            this->set_selected_index(std::max(m_selected_index - 1, 0));
            if (m_callback != nullptr)
                m_callback(m_selected_index);

            return true;
        }

        return ui::Widget::on_mouse_scroll(mouse, kb);
    }
}
