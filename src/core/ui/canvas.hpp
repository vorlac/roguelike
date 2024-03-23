#pragma once

#include <functional>
#include <string>
#include <vector>

#include "ds/dims.hpp"
#include "sdl/defs.hpp"
#include "utils/numeric.hpp"
#include "utils/time.hpp"
#include "widget.hpp"

namespace rl {
    class NVGRenderer;
}

SDL_C_LIB_BEGIN
#include <SDL3/SDL_video.h>
SDL_C_LIB_END

namespace rl::ui {
    using WindowID = SDL3::SDL_WindowID;
    using DisplayID = SDL3::SDL_DisplayID;

    using PixelFormat = i32;
    using ComponentFormat = i32;
    class ScrollableWidget;

    class Canvas final : public Widget
    {
    public:
        Canvas() = delete;
        Canvas(Canvas&&) = delete;
        Canvas(const Canvas&) = delete;
        Canvas& operator=(Canvas&&) = delete;
        Canvas& operator=(const Canvas&) = delete;

        enum class MouseMode {
            Propagate,  // Propagate mouse inputs to children widgets
            Ignore,     // Ignore all mouse inputs
            Resize,     // Resize floating children dialogs
            Drag        // Drag child widgets
        };

    public:
        explicit Canvas(const ds::rect<f32>& rect, const Mouse& mouse, const Keyboard& kb,
                        const std::unique_ptr<NVGRenderer>& nvg_renderer);
        virtual ~Canvas() override = default;

        bool draw_all();
        bool redraw();
        bool draw_widgets();
        bool update() const;
        bool draw_setup() const;
        bool draw_contents() const;
        bool draw_teardown() const;

        bool has_depth_buffer() const;
        bool has_stencil_buffer() const;
        bool has_float_buffer() const;
        bool tooltip_fade_in_progress();

        bool on_moved(ds::point<f32>&& pt);
        bool on_resized(ds::dims<f32>&& size);
        bool on_mouse_scroll_event(const Mouse& mouse, const Keyboard& kb);
        bool on_mouse_button_pressed_event(const Mouse& mouse, const Keyboard& kb);
        bool on_mouse_button_released_event(const Mouse& mouse, const Keyboard& kb);
        bool on_mouse_move_event(const Mouse& mouse, const Keyboard& kb);

        void center_dialog(ScrollableDialog* dialog) const;
        void dispose_dialog(ScrollableDialog* dialog);

        void move_dialog_to_front(ScrollableDialog* dialog);
        void update_focus(Widget* widget);
        void set_resize_callback(const std::function<void(ds::dims<f32>)>& callback);
        void add_update_callback(const std::function<void()>& refresh_func);
        void set_mouse_mode(MouseMode mouse_mode);

        using Widget::perform_layout;
        const std::function<void(ds::dims<f32>)>& resize_callback() const;
        ds::dims<i32> frame_buffer_size() const;
        ComponentFormat component_format() const;
        PixelFormat pixel_format() const;
        std::string title() const;

    public:
        virtual void set_visible(bool visible) override;
        virtual bool on_key_pressed(const Keyboard& kb) override;
        virtual bool on_key_released(const Keyboard& kb) override;
        virtual bool on_character_input(const Keyboard& kb) override;

    protected:
        ds::dims<i32> m_framebuf_size{ 0, 0 };
        std::vector<Widget*> m_focus_path{};
        std::string m_title{};

        f32 m_last_interaction{ 0.0f };
        f32 m_tooltip_delay{ 0.5f };
        f32 m_pixel_ratio{ 1.0f };

        bool m_redraw{ true };
        bool m_process_events{ true };

        const Mouse& m_mouse{};
        const Keyboard& m_keyboard{};

        std::function<void(ds::dims<f32>)> m_resize_callback;
        std::vector<std::function<void()>> m_update_callbacks;

    private:
        MouseMode m_mouse_mode{ MouseMode::Propagate };
        ScrollableDialog* m_active_dialog{ nullptr };
        Widget* m_active_widget{ nullptr };
    };
}
