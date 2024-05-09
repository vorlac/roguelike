#pragma once

#include <functional>

#include "ui/widgets/popupbutton.hpp"

namespace rl {
    class Mouse;
    class Keyboard;

    namespace ui {
        class Widget;
        class VerticalScrollPanel;

        class ComboBox final : public PopupButton
        {
        public:
            explicit ComboBox(Widget* parent);
            ComboBox(Widget* parent, const std::vector<std::string>& items);
            ComboBox(Widget* parent, const std::vector<std::string>& items,
                     const std::vector<std::string>& items_short);

            void set_selected_index(i32 idx);
            void set_callback(const std::function<void(u32)>& callback);
            void set_items(const std::vector<std::string>& items);
            void set_items(const std::vector<std::string>& items,
                           const std::vector<std::string>& items_short);

            i32 item_count() const;
            i32 selected_index() const;
            const std::vector<std::string>& items() const;
            const std::vector<std::string>& items_short() const;
            const std::function<void(u32)>& callback() const;

        public:
            virtual bool on_mouse_scroll(const Mouse& mouse, const Keyboard& kb) override;

        protected:
            VerticalScrollPanel* m_vscroll_panel{ nullptr };
            Widget* m_item_container{ nullptr };
            std::vector<std::string> m_items{};
            std::vector<std::string> m_items_short{};
            std::function<void(u32)> m_callback;
            i32 m_selected_index{ 0 };
        };
    }
}
