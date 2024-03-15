#pragma once

#include <string>

#include "core/ui/widget.hpp"
#include "ds/dims.hpp"

namespace rl {
    class Keyboard;
    class Mouse;

    namespace ui {
        class Popup;

        class Dialog : public Widget
        {
        public:
            friend class Popup;

            enum class Mode {
                None,     // constant positionioning
                Modal,    // scopes all GUI focus/input
                Move,     // being moved or can be moved
                Resizing  // being resized or can be resized
            };

        public:
            explicit Dialog(Widget* parent, std::string title = "Untitled Dialog");

            Dialog::Mode mode() const;
            std::string title() const;
            f32 header_height() const;
            Widget* button_panel();

            void set_title(const std::string& title);
            void set_mode(Dialog::Mode mode);
            void dispose();
            void center();

        public:
            virtual bool on_mouse_button_pressed(const Mouse& mouse, const Keyboard& kb) override;
            virtual bool on_mouse_button_released(const Mouse& mouse, const Keyboard& kb) override;
            virtual bool on_mouse_scroll(const Mouse& mouse, const Keyboard& kb) override;
            virtual bool on_mouse_drag(const Mouse& mouse, const Keyboard& kb) override;
            virtual bool on_mouse_entered(const Mouse& mouse) override;
            virtual bool on_mouse_exited(const Mouse& mouse) override;

        public:
            virtual void draw() override;
            virtual void perform_layout() override;
            virtual ds::dims<f32> preferred_size() const override;

        protected:
            virtual void refresh_relative_placement();

        protected:
            std::string m_title{};
            Widget* m_button_panel{ nullptr };
            Mode m_mode{ Mode::None };
            // bool m_modal{ false };
            // bool m_drag_move{ false };
            // bool m_drag_resize{ false };
        };
    }
}
