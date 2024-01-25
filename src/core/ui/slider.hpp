#include <functional>
#include <utility>

#include "core/ui/widget.hpp"
#include "ds/color.hpp"

namespace rl {
    class Mouse;
    class Keyboard;

    namespace ui {
        class Slider : public Widget
        {
        public:
            Slider(Widget* parent);

            f32 value() const;
            const ds::color<f32>& highlight_color() const;
            std::pair<f32, f32> range() const;
            std::pair<f32, f32> highlighted_range() const;
            const std::function<void(f32)>& callback() const;
            const std::function<void(f32)>& final_callback() const;

            void set_value(f32 value);
            void set_highlight_color(ds::color<f32> highlight_color);
            void set_range(std::pair<f32, f32> range);
            void set_highlighted_range(std::pair<f32, f32> highlighted_range);
            void set_callback(const std::function<void(f32)>& callback);
            void set_final_callback(const std::function<void(f32)>& callback);

        public:
            virtual ds::dims<f32> preferred_size() const override;
            virtual bool on_mouse_drag(const Mouse& mouse, const Keyboard& kb) override;
            virtual bool on_mouse_button_pressed(const Mouse& mouse, const Keyboard& kb) override;
            virtual bool on_mouse_button_released(const Mouse& mouse, const Keyboard& kb) override;
            virtual void draw() override;

        protected:
            f32 m_value{ 0.0f };
            std::function<void(f32)> m_callback;
            std::function<void(f32)> m_final_callback;
            std::pair<f32, f32> m_range{ 0.0f, 1.0f };
            std::pair<f32, f32> m_highlighted_range{ 0.0f, 0.0f };
            ds::color<f32> m_highlight_color{ 255, 80, 80, 70 };
        };

    }
}
