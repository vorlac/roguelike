#pragma once

#include <functional>
#include <string>
#include <vector>

#include "core/ui/widget.hpp"
#include "ds/dims.hpp"
#include "ds/point.hpp"
#include "ds/rect.hpp"
#include "sdl/defs.hpp"
#include "utils/numeric.hpp"
#include "utils/time.hpp"

namespace rl::ui {
    using WindowID = SDL3::SDL_WindowID;
    using DisplayID = SDL3::SDL_DisplayID;

    using PixelFormat = i32;
    using ComponentFormat = i32;

    class Canvas final : public Widget
    {
    public:
        Canvas(ds::rect<f32> rect, const Mouse& mouse, const Keyboard& kb,
               const std::unique_ptr<NVGRenderer>& nvg_renderer);
        ~Canvas() override;

        bool redraw();
        bool draw_widgets();

        void set_visible(bool visible);
        void center_dialog(Dialog* dialog) const;
        void move_dialog_to_front(Dialog* dialog);
        void update_focus(Widget* widget);
        void dispose_dialog(Dialog* dialog);
        void set_resize_callback(const std::function<void(ds::dims<f32>)>& callback);
        void add_update_callback(const std::function<void()>& refresh_func);
        const std::function<void(ds::dims<f32>)>& resize_callback() const;

        std::string title() const;
        ds::dims<i32> frame_buffer_size() const;

        static bool has_depth_buffer();
        static bool has_stencil_buffer();
        static bool has_float_buffer();
        bool tooltip_fade_in_progress() const;

        using Widget::perform_layout;
        static ComponentFormat component_format();
        static PixelFormat pixel_format();

    public:
        virtual bool update();
        virtual bool draw_all();
        virtual bool draw_setup();
        virtual bool draw_contents();
        virtual bool draw_teardown();

        virtual bool on_moved(const ds::point<f32>& pt);
        virtual bool on_resized(const ds::dims<f32>& size);

        bool on_key_pressed(const Keyboard& kb) override;
        bool on_key_released(const Keyboard& kb) override;
        bool on_character_input(const Keyboard& kb) override;

        bool on_mouse_entered(const Mouse& mouse) override;
        bool on_mouse_exited(const Mouse& mouse) override;
        bool on_mouse_scroll(const Mouse& mouse, const Keyboard& kb) override;
        bool on_mouse_button_pressed(const Mouse& mouse, const Keyboard& kb) override;
        bool on_mouse_button_released(const Mouse& mouse, const Keyboard& kb) override;
        bool on_mouse_move(const Mouse& mouse, const Keyboard& kb) override;
        bool on_mouse_drag(const Mouse& mouse, const Keyboard& kb) override;

        bool on_focus_gained() override;
        bool on_focus_lost() override;

    protected:
        ds::dims<i32> m_framebuf_size{ 0, 0 };
        std::vector<Widget*> m_focus_path{};
        Widget* m_drag_widget{ nullptr };
        std::string m_title{};

        f32 m_last_interaction{ 0.0f };
        f32 m_tooltip_delay{ 0.5f };
        f32 m_pixel_ratio{ 1.0f };

        bool m_redraw{ true };
        bool m_process_events{ true };
        bool m_drag_active{ false };

        const Mouse& m_mouse{};
        const Keyboard& m_keyboard{};

        std::function<void(ds::dims<f32>)> m_resize_callback;
        std::vector<std::function<void()>> m_update_callbacks;
        std::array<SDL3::SDL_Cursor*, Mouse::Cursor::CursorCount> m_cursors{};
    };
}
