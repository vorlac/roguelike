#pragma once

#include <functional>
#include <string>
#include <vector>

#include "core/ui/widget.hpp"
#include "ds/dims.hpp"
#include "ds/point.hpp"
#include "ds/rect.hpp"
#include "ds/vector2d.hpp"
#include "sdl/defs.hpp"
#include "utils/numeric.hpp"
#include "utils/time.hpp"

namespace rl {

    class Dialog;
    class Keyboard;
    class Mouse;

    namespace ui {
        using WindowID = SDL3::SDL_WindowID;
        using DisplayID = SDL3::SDL_DisplayID;

        using PixelFormat = i32;
        using ComponentFormat = i32;

        class Screen : public ui::widget
        {
        public:
            Screen(NVGcontext* nvg_context, ds::dims<i32> size, const Mouse& mouse,
                   const Keyboard& kb);
            ~Screen();

            bool redraw();
            bool draw_widgets();

            void center_window(Dialog* dialog) const;
            void move_window_to_front(Dialog* dialog);
            void update_focus(ui::widget* widget);
            void dispose_window(Dialog* dialog);

            void set_visible(bool visible);  // intentionally doesn't use virtual for this for this.
            void set_resize_callback(const std::function<void(ds::dims<i32>)>& callback);
            void add_refresh_callback(const std::function<void()>& refresh_func);
            void drop_callback_event(i32 count, const char** filenames);
            void nvg_flush();

            std::string title() const;
            ds::dims<i32> frame_buffer_size() const;
            NVGcontext* nvg_context() const;
            const ds::color<u8>& background() const;
            void set_background(ds::color<u8> background);
            const std::function<void(ds::dims<i32>)>& resize_callback() const;

            bool has_depth_buffer() const;
            bool has_stencil_buffer() const;
            bool has_float_buffer() const;
            bool tooltip_fade_in_progress() const;

            using ui::widget::perform_layout;
            void perform_layout();

            ComponentFormat component_format() const;
            PixelFormat pixel_format() const;

        public:
            virtual bool refresh();
            virtual bool draw_all();
            virtual bool draw_setup();
            virtual bool draw_contents();
            virtual bool draw_teardown();

            virtual bool on_key_pressed(const Keyboard& kb) override;
            virtual bool on_key_released(const Keyboard& kb) override;
            virtual bool on_character_input(const Keyboard& kb) override;

            virtual bool on_mouse_entered(const Mouse& mouse) override;
            virtual bool on_mouse_exited(const Mouse& mouse) override;
            virtual bool on_mouse_scroll(const Mouse& mouse, const Keyboard& kb) override;
            virtual bool on_mouse_button_pressed(const Mouse& mouse, const Keyboard& kb) override;
            virtual bool on_mouse_button_released(const Mouse& mouse, const Keyboard& kb) override;
            virtual bool on_mouse_move(const Mouse& mouse, const Keyboard& kb) override;
            virtual bool on_mouse_drag(const Mouse& mouse, const Keyboard& kb) override;

            virtual bool on_focus_gained() override;
            virtual bool on_focus_lost() override;

            virtual bool on_moved(WindowID id, ds::point<i32> pt);
            virtual bool on_resized(ds::dims<i32> size);
            virtual bool drop_event(const std::vector<std::string>& filenames);

        protected:
            ui::widget* m_drag_widget{ nullptr };
            NVGcontext* m_nvg_context{ nullptr };

            std::vector<ui::widget*> m_focus_path{};
            std::function<void(ds::dims<i32>)> m_resize_callback;
            std::vector<std::function<void()>> m_refresh_callbacks;
            std::array<SDL3::SDL_Cursor*, Mouse::Cursor::CursorCount> m_cursors{};

            std::string m_title{};
            ds::color<u8> m_background_color{ 29, 32, 39 };
            ds::dims<i32> m_framebuf_size{ 0, 0 };

            f32 m_last_interaction{ 0.0f };
            f32 m_tooltip_delay{ 0.5f };
            f32 m_pixel_ratio{ 1.0f };

            bool m_depth_buffer{ false };
            bool m_stencil_buffer{ false };
            bool m_float_buffer{ false };
            bool m_drag_active{ false };
            bool m_process_events{ true };
            bool m_redraw{ true };

            const Mouse& m_mouse_ref;
            const Keyboard& m_kb_ref;
        };
    }
}