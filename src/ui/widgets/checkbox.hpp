#pragma once

#include <functional>
#include <string>

#include "ui/widget.hpp"

namespace rl {
    class Mouse;
    class Keyboard;

    namespace ui {
        class CheckBox final : public Widget
        {
        public:
            explicit CheckBox(
                std::string text,
                const std::function<void(bool)>& toggled_callback = nullptr);
            explicit CheckBox(
                Widget* parent, std::string text,
                const std::function<void(bool)>& toggled_callback = nullptr);

            bool checked() const;
            bool pressed() const;
            std::string_view text() const;
            void set_checked(bool checked);
            void set_pressed(bool pressed);
            void set_text(std::string text);
            void set_callback(const std::function<void(bool)>& toggled_callback);
            const std::function<void(bool)>& callback() const;

        public:
            virtual void draw() override;
            virtual bool on_mouse_button_pressed(const Mouse& mouse, const Keyboard& kb, ds::point<f32> local_pos = {}) override;
            virtual bool on_mouse_button_released(const Mouse& mouse, const Keyboard& kb) override;
            virtual ds::dims<f32> preferred_size() const override;

        protected:
            std::string m_text{};
            bool m_pressed{ false };
            bool m_checked{ false };
            std::function<void(bool)> m_toggled_callback;

        private:
            constexpr static inline f32 CORNER_RADIUS{ 3.0f };
            constexpr static inline f32 OUTER_BLUR{ 3.0f };
            // the spacing offset between the cbeckbox rect
            // and the text label for the checkbox. represents
            // a perfentage of the checkbox square's height.
            constexpr static inline f32 TEXT_OFFSET{ 0.33f };
        };
    }
}
