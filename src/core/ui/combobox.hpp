#pragma once

#include <functional>

#include "core/ui/popupbutton.hpp"

namespace rl {
    class Mouse;
    class Keyboard;

    namespace ui {
        class Widget;
        class VScrollPanel;

        class ComboBox : public ui::PopupButton
        {
        public:
            ComboBox(ui::Widget* parent);
            ComboBox(ui::Widget* parent, const std::vector<std::string>& items);
            ComboBox(ui::Widget* parent, const std::vector<std::string>& items,
                     const std::vector<std::string>& items_short);

            void set_selected_index(i32 idx);
            void set_callback(const std::function<void(i32)>& callback);
            void set_items(const std::vector<std::string>& items);
            void set_items(const std::vector<std::string>& items,
                           const std::vector<std::string>& items_short);

            i32 selected_index() const;
            const std::vector<std::string>& items() const;
            const std::vector<std::string>& items_short() const;
            const std::function<void(int)>& callback() const;

        public:
            virtual bool on_mouse_scroll(const Mouse& mouse, const Keyboard& kb) override;

        protected:
            ui::VScrollPanel* m_vscroll_panel{ nullptr };
            ui::Widget* m_container{ nullptr };
            std::vector<std::string> m_items{};
            std::vector<std::string> m_items_short{};
            std::function<void(int)> m_callback;
            i32 m_selected_index{ 0 };
        };
    }
}