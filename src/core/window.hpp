#pragma once

#include <glad/gl.h>

#include <bitset>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "core/keyboard.hpp"
#include "core/mouse.hpp"
#include "core/ui/widget.hpp"
#include "ds/dims.hpp"
#include "ds/point.hpp"
#include "ds/rect.hpp"
#include "ds/vector2d.hpp"
#include "sdl/defs.hpp"
#include "utils/time.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_video.h>
SDL_C_LIB_END

namespace rl {
    class EventHandler;
    class Renderer;

    using WindowID = SDL3::SDL_WindowID;
    using DisplayID = SDL3::SDL_DisplayID;

    namespace ui {
        class Screen;
    }

    class Window
    {
        friend class Popup;

    public:
        // clang-format off
        struct Event
        {
            using Data = SDL3::SDL_WindowEvent;

            enum ID : std::underlying_type_t<SDL3::SDL_EventType> {
                WindowFirst         = SDL3::SDL_EVENT_WINDOW_FIRST,                  // The first Window::Event::ID possible (lowest integer value)
                Shown               = SDL3::SDL_EVENT_WINDOW_SHOWN,                  // Window has been shown
                Hidden              = SDL3::SDL_EVENT_WINDOW_HIDDEN,                 // Window has been hidden
                Exposed             = SDL3::SDL_EVENT_WINDOW_EXPOSED,                // Window has been exposed and should be redrawn
                Moved               = SDL3::SDL_EVENT_WINDOW_MOVED,                  // The window has been moved to point<i32>{x=data1, y=data2}
                Resized             = SDL3::SDL_EVENT_WINDOW_RESIZED,                // The window has been resized to dims<i32>{w:data1, h:data2}
                PixelSizeChanged    = SDL3::SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED,     // The pixel size of the window has changed to dims<i32>{w:data1, h:data2}
                Minimized           = SDL3::SDL_EVENT_WINDOW_MINIMIZED,              // Window has been minimized
                Maximized           = SDL3::SDL_EVENT_WINDOW_MAXIMIZED,              // Window has been maximized
                Restored            = SDL3::SDL_EVENT_WINDOW_RESTORED,               // Window has been restored to normal size and position
                MouseEnter          = SDL3::SDL_EVENT_WINDOW_MOUSE_ENTER,            // Window has gained mouse focus
                MouseLeave          = SDL3::SDL_EVENT_WINDOW_MOUSE_LEAVE,            // Window has lost mouse focus
                FocusGained         = SDL3::SDL_EVENT_WINDOW_FOCUS_GAINED,           // Window has gained keyboard focus
                FocusLost           = SDL3::SDL_EVENT_WINDOW_FOCUS_LOST,             // Window has lost keyboard focus
                CloseRequested      = SDL3::SDL_EVENT_WINDOW_CLOSE_REQUESTED,        // The window manager requests that the window be closed
                TakeFocus           = SDL3::SDL_EVENT_WINDOW_TAKE_FOCUS,             // Window is being offered a focus.  Should SetWindowInputFocus() on itself or a subwindow, or ignore
                HitTest             = SDL3::SDL_EVENT_WINDOW_HIT_TEST,               // Window had a hit test that wasn't SDL_HITTEST_NORMAL
                ICCProfChanged      = SDL3::SDL_EVENT_WINDOW_ICCPROF_CHANGED,        // The ICC profile of the window's display has changed
                DisplayChanged      = SDL3::SDL_EVENT_WINDOW_DISPLAY_CHANGED,        // Window has been moved to display: data1
                DisplayScaleChanged = SDL3::SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED,  // Window display scale has been changed
                Occluded            = SDL3::SDL_EVENT_WINDOW_OCCLUDED,               // The window has been occluded
                Destroyed           = SDL3::SDL_EVENT_WINDOW_DESTROYED,              // The window with the associated ID is being or has been destroyed.
                PenEnter            = SDL3::SDL_EVENT_WINDOW_PEN_ENTER,              // Window has gained focus of the pressure-sensitive pen with ID "data1"
                PenLeave            = SDL3::SDL_EVENT_WINDOW_PEN_LEAVE,              // Window has lost focus of the pressure-sensitive pen with ID "data1"
                WindowLast          = SDL3::SDL_EVENT_WINDOW_LAST,                   // The last Window::Event::ID possible (highest integer value)
            };
        };

        struct DisplayEvent
        {
            using Data = SDL3::SDL_DisplayEvent;

            enum ID : std::underlying_type_t<SDL3::SDL_EventType> {
                DisplayFirst        = SDL3::SDL_EVENT_DISPLAY_FIRST,                 // The first Window::DisplayEvent::ID possible (lowest integer value)
                Orientation         = SDL3::SDL_EVENT_DISPLAY_ORIENTATION,           // Display orientation has changed to data1
                Added               = SDL3::SDL_EVENT_DISPLAY_ADDED,                 // Display has been added to the system
                Removed             = SDL3::SDL_EVENT_DISPLAY_REMOVED,               // Display has been removed from the system
                Moved               = SDL3::SDL_EVENT_DISPLAY_MOVED,                 // Display has changed position
                ContentScaleChanged = SDL3::SDL_EVENT_DISPLAY_CONTENT_SCALE_CHANGED, // Display has changed content scale
                DisplayLast         = SDL3::SDL_EVENT_DISPLAY_LAST,                  // The last Window::DisplayEvent::ID possible (highest integer value)
            };
        };

        struct Properties : public std::bitset<sizeof(u32) * 8>
        {
            using sdl_type = SDL3::SDL_WindowFlags;

            enum Flag : std::underlying_type_t<sdl_type> {
                None            = 0,
                Fullscreen      = SDL3::SDL_WINDOW_FULLSCREEN,          // window is in fullscreen mode
                OpenGL          = SDL3::SDL_WINDOW_OPENGL,              // window usable with OpenGL context
                Occluded        = SDL3::SDL_WINDOW_OCCLUDED,            // window is occluded
                Hidden          = SDL3::SDL_WINDOW_HIDDEN,              // window is neither mapped onto the desktop nor shown in the taskbar/dock/window list; SDL_ShowWindow() is required for it to become visible
                Borderless      = SDL3::SDL_WINDOW_BORDERLESS,          // no window decoration
                Resizable       = SDL3::SDL_WINDOW_RESIZABLE,           // window can be resized
                Minimized       = SDL3::SDL_WINDOW_MINIMIZED,           // window is minimized
                Maximized       = SDL3::SDL_WINDOW_MAXIMIZED,           // window is maximized
                MouseGrabbed    = SDL3::SDL_WINDOW_MOUSE_GRABBED,       // window has grabbed mouse input
                InputFocus      = SDL3::SDL_WINDOW_INPUT_FOCUS,         // window has input focus
                MouseFocus      = SDL3::SDL_WINDOW_MOUSE_FOCUS,         // window has mouse focus
                External        = SDL3::SDL_WINDOW_EXTERNAL,            // window not created by SDL
                HighDPI         = SDL3::SDL_WINDOW_HIGH_PIXEL_DENSITY,  // window uses high pixel density back buffer if possible
                MouseCapture    = SDL3::SDL_WINDOW_MOUSE_CAPTURE,       // window has mouse captured (unrelated to MOUSE_GRABBED)
                AlwaysOnTop     = SDL3::SDL_WINDOW_ALWAYS_ON_TOP,       // window should always be above others
                Utility         = SDL3::SDL_WINDOW_UTILITY,             // window should be treated as a utility window, not showing in the task bar and window list
                Tooltip         = SDL3::SDL_WINDOW_TOOLTIP,             // window should be treated as a tooltip
                PopupMenu       = SDL3::SDL_WINDOW_POPUP_MENU,          // window should be treated as a popup menu
                KeyboardGrabbed = SDL3::SDL_WINDOW_KEYBOARD_GRABBED,    // window has grabbed keyboard input
                Vulkan          = SDL3::SDL_WINDOW_VULKAN,              // window usable for Vulkan surface
                Metal           = SDL3::SDL_WINDOW_METAL,               // window usable for Metal view
                Transparent     = SDL3::SDL_WINDOW_TRANSPARENT,         // window with transparent buffer
                NotFocusable    = SDL3::SDL_WINDOW_NOT_FOCUSABLE,       // window should not be focusable
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

        struct OpenGL
        {
            using type = SDL3::SDL_GLattr;
             
            enum Attribute : std::underlying_type_t<type> {
                RedSize                  = SDL3::SDL_GL_RED_SIZE,
                GreenSize                = SDL3::SDL_GL_GREEN_SIZE,
                BlueSize                 = SDL3::SDL_GL_BLUE_SIZE,
                AlphaSize                = SDL3::SDL_GL_ALPHA_SIZE,
                BufferSize               = SDL3::SDL_GL_BUFFER_SIZE,
                Doublebuffer             = SDL3::SDL_GL_DOUBLEBUFFER,
                DepthSize                = SDL3::SDL_GL_DEPTH_SIZE,
                StencilSize              = SDL3::SDL_GL_STENCIL_SIZE,
                AccumRedSize             = SDL3::SDL_GL_ACCUM_RED_SIZE,
                AccumGreenSize           = SDL3::SDL_GL_ACCUM_GREEN_SIZE,
                AccumBlueSize            = SDL3::SDL_GL_ACCUM_BLUE_SIZE,
                AccumAlphaSize           = SDL3::SDL_GL_ACCUM_ALPHA_SIZE,
                Stereo                   = SDL3::SDL_GL_STEREO,
                Multisamplebuffers       = SDL3::SDL_GL_MULTISAMPLEBUFFERS,
                Multisamplesamples       = SDL3::SDL_GL_MULTISAMPLESAMPLES,
                AcceleratedVisual        = SDL3::SDL_GL_ACCELERATED_VISUAL,
                RetainedBacking          = SDL3::SDL_GL_RETAINED_BACKING,
                ContextMajorVersion      = SDL3::SDL_GL_CONTEXT_MAJOR_VERSION,
                ContextMinorVersion      = SDL3::SDL_GL_CONTEXT_MINOR_VERSION,
                ContextFlags             = SDL3::SDL_GL_CONTEXT_FLAGS,
                ContextProfileMask       = SDL3::SDL_GL_CONTEXT_PROFILE_MASK,
                ShareWithCurrentContext  = SDL3::SDL_GL_SHARE_WITH_CURRENT_CONTEXT,
                FramebufferSrgbCapable   = SDL3::SDL_GL_FRAMEBUFFER_SRGB_CAPABLE,
                ContextReleaseBehavior   = SDL3::SDL_GL_CONTEXT_RELEASE_BEHAVIOR,
                ContextResetNotification = SDL3::SDL_GL_CONTEXT_RESET_NOTIFICATION,
                ContextNoError           = SDL3::SDL_GL_CONTEXT_NO_ERROR,
                Floatbuffers             = SDL3::SDL_GL_FLOATBUFFERS,
                EGLPlatform              = SDL3::SDL_GL_EGL_PLATFORM
            };

            enum ContextFlag : std::underlying_type_t<SDL3::SDL_GLcontextFlag> {
                Debug              = SDL3::SDL_GL_CONTEXT_DEBUG_FLAG,
                ForwardCompatible  = SDL3::SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG,
                RobustAccess       = SDL3::SDL_GL_CONTEXT_ROBUST_ACCESS_FLAG,
                ResetIsolation     = SDL3::SDL_GL_CONTEXT_RESET_ISOLATION_FLAG,
            };

            enum Profile : std::underlying_type_t<SDL3::SDL_GLprofile> {
                Core           = SDL3::SDL_GL_CONTEXT_PROFILE_CORE,
                Compatibility  = SDL3::SDL_GL_CONTEXT_PROFILE_COMPATIBILITY,
                ES             = SDL3::SDL_GL_CONTEXT_PROFILE_ES,
            };
        };

        // clang-format on

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
        explicit Window(std::string title, const ds::dims<i32>& dims = DEFAULT_SIZE,
                        Properties flags = DEFAULT_PROPERTY_FLAGS);

        virtual ~Window();

        const Window& operator=(Window&& other) noexcept;

    public:
        bool raise();
        bool restore();
        bool minimize();
        bool maximize();
        bool hide();
        bool show();
        bool swap_buffers();

        bool clear();
        bool render_start();
        bool render();
        bool render_end();

        std::string get_title();
        DisplayID get_display();
        ui::widget* button_panel();
        ds::dims<i32> get_size();
        ds::dims<i32> get_render_size();
        ds::dims<i32> get_min_size() const;
        ds::dims<i32> get_max_size() const;
        ds::point<i32> get_position() const;
        f32 get_opacity() const;

        SDL3::SDL_Window* sdl_handle() const;
        SDL3::SDL_WindowFlags get_flags() const;
        SDL3::SDL_DisplayMode get_display_mode() const;

        const std::unique_ptr<Renderer>& renderer() const;
        NVGcontext* nvg_context() const;

        const Keyboard& keyboard() const;
        const Mouse& mouse() const;
        ui::Screen* gui() const;

        bool is_valid() const;
        bool get_grab() const;

        bool set_vsync(bool enabled);
        bool set_grab(bool grabbed);
        bool set_bordered(bool bordered);
        bool set_resizable(bool resizable);
        bool set_fullscreen(bool fullscreen);
        bool set_opacity(float opacity);
        bool set_position(ds::point<i32> pos);
        bool set_size(ds::dims<i32> size);
        bool set_min_size(ds::dims<i32> size);
        bool set_max_size(ds::dims<i32> size);
        bool set_opengl_attribute(OpenGL::Attribute attr, auto val);
        bool set_background(ds::color<u8> background);
        bool on_occluded(const WindowID id);

    public:
        void mouse_moved_event_callback(const SDL3::SDL_Event& e);
        void mouse_wheel_event_callback(const SDL3::SDL_Event& e);
        void mouse_button_pressed_event_callback(const SDL3::SDL_Event& e);
        void mouse_button_released_event_callback(const SDL3::SDL_Event& e);
        void mouse_entered_event_callback(const SDL3::SDL_Event& e);
        void mouse_exited_event_callback(const SDL3::SDL_Event& e);

        void keyboard_key_pressed_event_callback(const SDL3::SDL_Event& e);
        void keyboard_key_released_event_callback(const SDL3::SDL_Event& e);
        void keyboard_char_event_callback(const SDL3::SDL_Event& e);
        void window_resized_event_callback(const SDL3::SDL_Event& e);
        void window_focus_gained_event_callback(const SDL3::SDL_Event& e);
        void window_focus_lost_event_callback(const SDL3::SDL_Event& e);

        bool on_pixel_size_changed(const WindowID id, ds::dims<i32> pixel_size);
        bool set_title(std::string title);
        bool set_modal(bool modal);

    private:
        Window(const Window& window) = delete;
        Window(Window&& window) noexcept = delete;
        Window(SDL3::SDL_Window* other) = delete;

    private:
        bool m_vsync{ true };

        std::string m_title{};
        WindowID m_window_id{ 0 };
        DisplayID m_display_id{ 0 };
        Properties m_properties{ Properties::None };
        ds::rect<i32> m_window_rect{ 0, 0, 0, 0 };
        ui::Screen* m_screen{ nullptr };
        SDL3::SDL_Window* m_sdl_window{ nullptr };
        ds::dims<i32> m_framebuf_size{ 0, 0 };
        std::unique_ptr<Renderer> m_renderer;
        Keyboard m_keyboard{};
        Mouse m_mouse{};

        // The content display scale relative to a window's pixel size.
        //
        // This is a combination of the window pixel density and the display content scale,
        // and is the expected scale for displaying content in this window. For example, if a
        // 3840x2160 window had a display scale of 2.0, the user expects the content to take
        // twice as many pixels and be the same physical size as if it were being displayed in
        // a 1920x1080 window with a display scale of 1.0. Conceptually this value corresponds
        // to the scale display setting, and is updated when that setting is changed, or the
        // window moves to a display with a different scale setting.
        f32 m_pixel_ratio{ 1.0f };

        // The pixel density of a window.
        //
        // This is a ratio of pixel size to window size.
        // For example, if the window is 1920x1080 and it has a high density
        // back buffer of 3840x2160 pixels, it would have a pixel density of 2.0.
        f32 m_pixel_density{ 1.0f };
    };

    // bool Screen::on_shown(const WindowID id)
    //{
    //     this->set_visible(true);
    //     if constexpr (io::logging::window_events)
    //         log::info("window::on_shown [id:{}]", id);

    //    return true;
    //}

    // bool Screen::on_hidden(const WindowID id)
    //{
    //     this->set_visible(false);
    //     if constexpr (io::logging::window_events)
    //         log::info("window::on_hidden [id:{}]", id);

    //    return true;
    //}

    // bool Screen::on_exposed(const WindowID id)
    //{
    //     m_visible = true;
    //     if constexpr (io::logging::window_events)
    //         log::info("window::on_exposed [id:{}]", id);

    //    return true;
    //}
    // bool Screen::on_minimized(const WindowID id)
    //{
    //     this->set_visible(false);
    //     if constexpr (io::logging::window_events)
    //         log::info("window::on_minimized [id:{}]", id);
    //     return true;
    // }

    // bool Screen::on_maximized(const WindowID id)
    //{
    //     bool ret = true;
    //     if constexpr (io::logging::window_events)
    //         log::info("window::on_maximized [id:{}]", id);
    //     return ret;
    // }

    // bool Screen::on_restored(const WindowID id)
    //{
    //     bool ret = true;
    //     if constexpr (io::logging::window_events)
    //         log::info("window::on_restored [id:{}]", id);
    //     return ret;
    // }

    // bool Screen::on_close_requested(const WindowID id)
    //{
    //     bool ret = true;
    //     if constexpr (io::logging::window_events)
    //         log::info("window::on_close_requested [id:{}]", id);
    //     return ret;
    // }

    // bool Screen::on_take_focus(const WindowID id)
    //{
    //     bool ret = true;
    //     if constexpr (io::logging::window_events)
    //         log::info("window::on_take_focus [id:{}]", id);
    //     return ret;
    // }

    // bool Screen::on_hit_test(const WindowID id)
    //{
    //     bool ret = true;
    //     if constexpr (io::logging::window_events)
    //         log::info("window::on_hit_test [id:{}]", id);
    //     return ret;
    // }

    // bool Screen::on_icc_profile_changed(const WindowID id)
    //{
    //     bool ret = true;
    //     if constexpr (io::logging::window_events)
    //         log::info("window::on_icc_profile_changed [id:{}]", id);
    //     return ret;
    // }

    // bool Screen::on_display_changed(const WindowID id)
    //{
    //     bool ret = true;
    //     if constexpr (io::logging::window_events)
    //         log::info("window::on_display_changed [id:{}]", id);
    //     return ret;
    // }
}
