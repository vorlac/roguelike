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

    class Window;
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
            Screen(NVGcontext* nvg_context);

            // external api impl methods
            //--------------------------
            bool init();
            bool redraw();
            bool draw_widgets();

            void update_focus(ui::widget* widget);
            void dispose_window(Window* window);
            void move_window_to_front(Window* window);
            void center_window(Window* window) const;

        public:
            // bool on_shown(const WindowID id);
            // bool on_hidden(const WindowID id);
            // bool on_exposed(const WindowID id);
            // bool on_moved(const WindowID id, ds::point<i32> pt);
            // bool on_resized(ds::dims<i32> size);
            // bool on_minimized(const WindowID id);
            // bool on_maximized(const WindowID id);
            // bool on_restored(const WindowID id);
            // bool on_close_requested(const WindowID id);
            // bool on_take_focus(const WindowID id);
            // bool on_hit_test(const WindowID id);
            // bool on_icc_profile_changed(const WindowID id);
            // bool on_display_changed(const WindowID id);
            // bool on_display_scale_changed(const WindowID id);
            // bool on_display_content_scale_changed(const DisplayID id);
            // bool on_occluded(const WindowID id);
            // bool on_destroyed(const WindowID id);

            void set_visible(bool visible);  // intentionally doesn't use virtual for this for this.
            void set_resize_callback(const std::function<void(ds::dims<i32>)>& callback);
            void add_refresh_callback(const std::function<void()>& refresh_func);
            void drop_callback_event(i32 count, const char** filenames);
            void nvg_flush();

            ds::point<i32> mouse_pos() const;
            ds::dims<i32> frame_buffer_size() const;
            NVGcontext* nvg_context() const;  // TODO: move to renderer
            const ds::color<u8>& background() const;
            void set_background(ds::color<u8> background);
            const std::function<void(ds::dims<i32>)>& resize_callback() const;

            // replaced with Window::title & Window::set_title, but may still need them
            // ------------------------------------------------------------------------
            // const std::string& caption() const;
            // void set_caption(const std::string& caption);

            // placeholders, needed for platform independent impl only
            bool has_depth_buffer() const;
            bool has_stencil_buffer() const;
            bool has_float_buffer() const;
            bool tooltip_fade_in_progress() const;
            void perform_layout();

            ComponentFormat component_format() const;
            PixelFormat pixel_format() const;

        public:
            friend class rl::Window;

            virtual void draw(NVGcontext* ctx) override;

            virtual bool draw_all();
            virtual bool draw_setup();
            virtual bool draw_contents();
            virtual bool draw_teardown();

            // virtual bool clear();
            virtual bool refresh();

            virtual bool drop_event(const std::vector<std::string>& filenames);

            virtual bool on_focus_gained() override;
            virtual bool on_focus_lost() override;

            virtual bool on_key_pressed(const Keyboard& kb) override;
            virtual bool on_key_released(const Keyboard& kb) override;
            virtual bool on_character_input(const Keyboard& kb) override;

            virtual bool on_mouse_entered(const Mouse& mouse) override;
            virtual bool on_mouse_exited(const Mouse& mouse) override;

            virtual bool on_mouse_scroll(const Mouse& mouse, const Keyboard& kb) override;
            virtual bool on_mouse_button_pressed(const Mouse& mouse, const Keyboard& kb) override;
            virtual bool on_mouse_button_released(const Mouse& mouse, const Keyboard& kb) override;

            virtual bool on_mouse_move(const Mouse& mouse, const Keyboard& kb) override;
            virtual bool on_mouse_drag(ds::point<i32> pnt, ds::vector2<i32> rel, const Mouse& mouse,
                                       const Keyboard& kb) override;

        protected:
            std::string m_title{};
            std::vector<ui::widget*> m_focus_path{};
            std::function<void(ds::dims<i32>)> m_resize_callback;
            std::vector<std::function<void()>> m_refresh_callbacks;
            ui::widget* m_drag_widget{ nullptr };
            NVGcontext* m_nvg_context{ nullptr };

            f32 m_last_interaction{ 0.0f };

            bool m_depth_buffer{ false };
            bool m_stencil_buffer{ false };
            bool m_float_buffer{ false };
            bool m_drag_active{ false };
            bool m_process_events{ true };
            bool m_redraw{ true };
        };
    }
}
