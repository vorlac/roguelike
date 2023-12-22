#pragma once
#include <glad/gl.h>

#include <bitset>
#include <memory>
#include <string>

#include "ds/dims.hpp"
#include "ds/point.hpp"
#include "ds/rect.hpp"
#include "ds/vector2d.hpp"
#include "gui/screen.hpp"
#include "sdl/defs.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_video.h>
SDL_C_LIB_END

namespace rl {
    class EventHandler;
    class Renderer;

    using WindowID = SDL3::SDL_WindowID;

    class Window : public gui::Screen
    {
    public:
        struct Event
        {
            using Data = SDL3::SDL_WindowEvent;

            enum ID : std::underlying_type_t<SDL3::SDL_EventType> {
                WindowFirst = SDL3::SDL_EVENT_WINDOW_FIRST,
                Shown = SDL3::SDL_EVENT_WINDOW_SHOWN,
                Hidden = SDL3::SDL_EVENT_WINDOW_HIDDEN,
                Exposed = SDL3::SDL_EVENT_WINDOW_EXPOSED,
                Moved = SDL3::SDL_EVENT_WINDOW_MOVED,
                Resized = SDL3::SDL_EVENT_WINDOW_RESIZED,
                PixelSizeChanged = SDL3::SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED,
                Minimized = SDL3::SDL_EVENT_WINDOW_MINIMIZED,
                Maximized = SDL3::SDL_EVENT_WINDOW_MAXIMIZED,
                Restored = SDL3::SDL_EVENT_WINDOW_RESTORED,
                MouseEnter = SDL3::SDL_EVENT_WINDOW_MOUSE_ENTER,
                MouseLeave = SDL3::SDL_EVENT_WINDOW_MOUSE_LEAVE,
                FocusGained = SDL3::SDL_EVENT_WINDOW_FOCUS_GAINED,
                FocusLost = SDL3::SDL_EVENT_WINDOW_FOCUS_LOST,
                CloseRequested = SDL3::SDL_EVENT_WINDOW_CLOSE_REQUESTED,
                TakeFocus = SDL3::SDL_EVENT_WINDOW_TAKE_FOCUS,
                HitTest = SDL3::SDL_EVENT_WINDOW_HIT_TEST,
                ICCProfChanged = SDL3::SDL_EVENT_WINDOW_ICCPROF_CHANGED,
                DisplayChanged = SDL3::SDL_EVENT_WINDOW_DISPLAY_CHANGED,
                DisplayScaleChanged = SDL3::SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED,
                Occluded = SDL3::SDL_EVENT_WINDOW_OCCLUDED,
                Destroyed = SDL3::SDL_EVENT_WINDOW_DESTROYED,
                PenEnter = SDL3::SDL_EVENT_WINDOW_PEN_ENTER,
                PenLeave = SDL3::SDL_EVENT_WINDOW_PEN_LEAVE,
                WindowLast = SDL3::SDL_EVENT_WINDOW_LAST,
            };
        };

        struct Properties : public std::bitset<sizeof(u32) * 8>
        {
            using sdl_type = SDL3::SDL_WindowFlags;

            enum Flag : std::underlying_type_t<sdl_type> {
                None = 0,
                Fullscreen = SDL3::SDL_WINDOW_FULLSCREEN,
                OpenGL = SDL3::SDL_WINDOW_OPENGL,
                Occluded = SDL3::SDL_WINDOW_OCCLUDED,
                Hidden = SDL3::SDL_WINDOW_HIDDEN,
                Borderless = SDL3::SDL_WINDOW_BORDERLESS,
                Resizable = SDL3::SDL_WINDOW_RESIZABLE,
                Minimized = SDL3::SDL_WINDOW_MINIMIZED,
                Maximized = SDL3::SDL_WINDOW_MAXIMIZED,
                MouseGrabbed = SDL3::SDL_WINDOW_MOUSE_GRABBED,
                InputFocus = SDL3::SDL_WINDOW_INPUT_FOCUS,
                MouseFocus = SDL3::SDL_WINDOW_MOUSE_FOCUS,
                External = SDL3::SDL_WINDOW_EXTERNAL,
                HighDPI = SDL3::SDL_WINDOW_HIGH_PIXEL_DENSITY,
                MouseCapture = SDL3::SDL_WINDOW_MOUSE_CAPTURE,
                AlwaysOnTop = SDL3::SDL_WINDOW_ALWAYS_ON_TOP,
                Utility = SDL3::SDL_WINDOW_UTILITY,
                Tooltip = SDL3::SDL_WINDOW_TOOLTIP,
                PopupMenu = SDL3::SDL_WINDOW_POPUP_MENU,
                KeyboardGrabbed = SDL3::SDL_WINDOW_KEYBOARD_GRABBED,
                Vulkan = SDL3::SDL_WINDOW_VULKAN,
                Metal = SDL3::SDL_WINDOW_METAL,
                Transparent = SDL3::SDL_WINDOW_TRANSPARENT,
                NotFocusable = SDL3::SDL_WINDOW_NOT_FOCUSABLE,
            };

            constexpr inline operator sdl_type()
            {
                return static_cast<sdl_type>(this->to_ulong());
            }

            constexpr inline operator sdl_type() const
            {
                return static_cast<sdl_type>(this->to_ulong());
            }
        };

        constexpr static inline Window::Properties DEFAULT_PROPERTY_FLAGS = {
            Properties::Flag::Resizable | Properties::Flag::OpenGL
        };

        constexpr static inline ds::point<i32> DEFAULT_POSITION = {
            SDL_WINDOWPOS_CENTERED_MASK,
            SDL_WINDOWPOS_CENTERED_MASK,
        };

        constexpr static inline ds::dims<i32> DEFAULT_SIZE = {
            1920,
            1080,
        };

    public:
        explicit Window(Window&& window) noexcept = delete;
        explicit Window(const Window& window) = delete;
        explicit Window(SDL3::SDL_Window* other) = delete;

        explicit Window(std::string title, const ds::dims<i32>& dims = DEFAULT_SIZE,
                        Properties flags = DEFAULT_PROPERTY_FLAGS);

        ~Window();

    public:
        bool maximize();
        bool minimize();
        bool hide();
        bool restore();
        bool raise();
        bool show();

        bool set_grab(bool grabbed);
        bool set_bordered(bool bordered);
        bool set_resizable(bool resizable);
        bool set_fullscreen(bool fullscreen);
        bool set_opacity(float opacity);
        bool set_title(std::string title);
        bool set_position(ds::point<i32> pos);
        bool set_size(ds::dims<i32> size);
        bool set_min_size(ds::dims<i32> size);
        bool set_max_size(ds::dims<i32> size);

        bool get_grab() const;
        f32 get_opacity() const;
        std::string get_title() const;
        SDL3::SDL_DisplayID get_display() const;
        SDL3::SDL_WindowFlags get_flags() const;
        SDL3::SDL_DisplayMode get_display_mode() const;
        ds::dims<i32> get_size() const;
        ds::dims<i32> get_render_size() const;
        ds::dims<i32> get_min_size() const;
        ds::dims<i32> get_max_size() const;
        ds::point<i32> get_position() const;

        const std::unique_ptr<Renderer>& renderer() const;
        const Window& operator=(Window&& other) noexcept;
        SDL3::SDL_Window* sdl_handle() const;
        bool is_valid() const;
        bool swap_buffers();

    protected:
        friend class EventHandler;

        bool on_shown(const WindowID id);
        bool on_hidden(const WindowID id);
        bool on_exposed(const WindowID id);
        bool on_moved(const WindowID id, ds::point<i32>&& pt);
        bool on_resized(const WindowID id, ds::dims<i32>&& size);
        bool on_pixel_size_changed(const WindowID id);
        bool on_minimized(const WindowID id);
        bool on_maximized(const WindowID id);
        bool on_restored(const WindowID id);
        bool on_mouse_enter(const WindowID id);
        bool on_mouse_leave(const WindowID id);
        bool on_kb_focus_gained(const WindowID id);
        bool on_kb_focus_lost(const WindowID id);
        bool on_close_requested(const WindowID id);
        bool on_take_focus(const WindowID id);
        bool on_hit_test(const WindowID id);
        bool on_icc_profile_changed(const WindowID id);
        bool on_display_changed(const WindowID id);
        bool on_display_scale_changed(const WindowID id);
        bool on_occluded(const WindowID id);
        bool on_destroyed(const WindowID id);

    private:
        Properties m_properties{ Properties::Flag::None };
        SDL3::SDL_Window* m_sdl_window{ nullptr };
        ds::rect<i32> m_window_rect{ 0, 0, 0, 0 };
        std::unique_ptr<rl::Renderer> m_renderer;
    };
}
