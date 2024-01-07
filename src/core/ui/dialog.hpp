#pragma once

#include <string>

#include "core/ui/screen.hpp"
#include "core/ui/widget.hpp"
#include "ds/dims.hpp"

namespace rl {
    class Keyboard;
    class Mouse;

    namespace ui {

        class Dialog : public ui::widget
        {
            friend class Popup;

        public:
            Dialog(ui::widget* parent, const std::string& title = "Untitled Dialog");

            void dispose();
            void center();

            bool modal() const;
            ui::widget* button_panel();
            const std::string& title() const;

            void set_title(const std::string& title);
            void set_modal(bool modal);

        public:
            virtual bool on_mouse_button_pressed(const Mouse& mouse, const Keyboard& kb) override;
            virtual bool on_mouse_button_released(const Mouse& mouse, const Keyboard& kb) override;
            virtual bool on_mouse_scroll(const Mouse& mouse, const Keyboard& kb) override;
            virtual bool on_mouse_drag(const Mouse& mouse, const Keyboard& kb) override;
            virtual bool on_mouse_entered(const Mouse& mouse) override;
            virtual bool on_mouse_exited(const Mouse& mouse) override;

        public:
            virtual void draw(NVGcontext* ctx) override;
            virtual void perform_layout(NVGcontext* ctx) override;
            virtual ds::dims<i32> preferred_size(NVGcontext* nvg_context) const override;

        protected:
            virtual void refresh_relative_placement();

        protected:
            std::string m_title{ "" };
            ui::widget* m_button_panel{ nullptr };
            bool m_modal{ false };
            bool m_drag{ false };
        };
    }
}
