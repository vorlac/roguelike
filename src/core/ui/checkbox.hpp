#pragma once

#include <functional>
#include <string>

#include "core/ui/widget.hpp"
#include "ds/dims.hpp"

namespace rl {
    class Mouse;
    class Keyboard;

    namespace ui {
        class CheckBox : public Widget
        {
        public:
            explicit CheckBox(
                Widget* parent, std::string caption = "UntitledCB",
                const std::function<void(bool)>& toggled_callback = std::function<void(bool)>());

            const bool& checked() const;
            const bool& pushed() const;
            const std::string& caption() const;

            void set_checked(const bool& checked);
            void set_pushed(const bool& pushed);
            void set_caption(const std::string& caption);

            const std::function<void(bool)>& callback() const;
            void set_callback(const std::function<void(bool)>& toggled_callback);

        public:
            bool on_mouse_button_pressed(const Mouse& mouse, const Keyboard& kb) override;
            bool on_mouse_button_released(const Mouse& mouse, const Keyboard& kb) override;

            ds::dims<f32> preferred_size() const override;
            void draw() override;

        protected:
            bool m_pushed{ false };
            bool m_checked{ false };
            std::string m_caption{};
            std::function<void(bool)> m_toggled_callback;
        };
    }
}
