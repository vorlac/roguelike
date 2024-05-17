#pragma once

#include <functional>
#include <utility>
#include <vector>

#include "ui/widget.hpp"

namespace rl {
    class Mouse;
    class Keyboard;

    namespace ui {
        class ImagePanel : public Widget
        {
        public:
            using Images = std::vector<std::pair<i32, std::string>>;

        public:
            explicit ImagePanel(Widget* parent);

            void set_images(const Images& data);
            const Images& images() const;
            const std::function<void(int)>& callback() const;
            void set_callback(const std::function<void(int)>& callback);

        public:
            virtual bool on_mouse_move(const Mouse& mouse, const Keyboard& kb) override;
            virtual bool on_mouse_button_pressed(const Mouse& mouse, const Keyboard& kb, ds::point<f32> local_pos = {}) override;

            virtual ds::dims<f32> preferred_size() const override;
            virtual void draw() override;

        protected:
            ds::dims<i32> grid_size() const;
            i32 index_for_position(const ds::point<f32>& mouse_pos) const;

        protected:
            Images m_images{};
            std::function<void(i32)> m_callback;
            ds::dims<f32> m_thumb_size{ 64.0f, 64.0f };
            ds::vector2<f32> m_spacing{ 10.0f, 10.0f };
            ds::vector2<f32> m_margin{ 10.0f, 10.0f };
            i32 m_mouse_index{ -1 };
        };
    }
}
