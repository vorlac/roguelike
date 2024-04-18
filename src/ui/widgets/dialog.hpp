#pragma once

#include <string>

#include "ds/dims.hpp"
#include "ui/widget.hpp"

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
            Side resize_side() const;
            f32 header_height() const;
            Widget* button_panel();

            void center();
            void set_resize_grab_pos(Side side);
            void set_title(const std::string& title);
            void set_mode(Dialog::Mode mode);
            void dispose();

        public:
            virtual bool on_mouse_button_pressed(const Mouse& mouse, const Keyboard& kb) override;
            virtual bool on_mouse_button_released(const Mouse& mouse, const Keyboard& kb) override;
            virtual bool on_mouse_scroll(const Mouse& mouse, const Keyboard& kb) override;
            virtual bool on_mouse_drag(const Mouse& mouse, const Keyboard& kb) override;

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
            Side m_resize_grab_location{ Side::None };
            // bool m_modal{ false };
        };
    }
}
