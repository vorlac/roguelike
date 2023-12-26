#pragma once
#include <glad/gl.h>

#include <bitset>
#include <functional>
#include <memory>
#include <string>
#include <vector>

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

    class Window : public ui::widget
    {
    public:
        struct Event
        {
            using Data = SDL3::SDL_WindowEvent;

            enum ID : std::underlying_type_t<SDL3::SDL_EventType> {
                // The first Window::Event::ID possible (lowest integer value)
                WindowFirst = SDL3::SDL_EVENT_WINDOW_FIRST,
                // Window has been shown
                Shown = SDL3::SDL_EVENT_WINDOW_SHOWN,
                // Window has been hidden
                Hidden = SDL3::SDL_EVENT_WINDOW_HIDDEN,
                // Window has been exposed and should be redrawn
                Exposed = SDL3::SDL_EVENT_WINDOW_EXPOSED,
                // The window has been moved to point<i32>{x=data1, y=data2}
                Moved = SDL3::SDL_EVENT_WINDOW_MOVED,
                // The window has been resized to dims<i32>{w:data1, h:data2}
                Resized = SDL3::SDL_EVENT_WINDOW_RESIZED,
                // The pixel size of the window has changed to dims<i32>{w:data1, h:data2}
                PixelSizeChanged = SDL3::SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED,
                // Window has been minimized
                Minimized = SDL3::SDL_EVENT_WINDOW_MINIMIZED,
                // Window has been maximized
                Maximized = SDL3::SDL_EVENT_WINDOW_MAXIMIZED,
                // Window has been restored to normal size and position
                Restored = SDL3::SDL_EVENT_WINDOW_RESTORED,
                // Window has gained mouse focus
                MouseEnter = SDL3::SDL_EVENT_WINDOW_MOUSE_ENTER,
                // Window has lost mouse focus
                MouseLeave = SDL3::SDL_EVENT_WINDOW_MOUSE_LEAVE,
                // Window has gained keyboard focus
                FocusGained = SDL3::SDL_EVENT_WINDOW_FOCUS_GAINED,
                // Window has lost keyboard focus
                FocusLost = SDL3::SDL_EVENT_WINDOW_FOCUS_LOST,
                // The window manager requests that the window be closed
                CloseRequested = SDL3::SDL_EVENT_WINDOW_CLOSE_REQUESTED,
                // Window is being offered a focus.
                // Should SetWindowInputFocus() on itself or a subwindow, or ignore
                TakeFocus = SDL3::SDL_EVENT_WINDOW_TAKE_FOCUS,
                // Window had a hit test that wasn't SDL_HITTEST_NORMAL
                HitTest = SDL3::SDL_EVENT_WINDOW_HIT_TEST,
                // The ICC profile of the window's display has changed
                ICCProfChanged = SDL3::SDL_EVENT_WINDOW_ICCPROF_CHANGED,
                // Window has been moved to display: data1
                DisplayChanged = SDL3::SDL_EVENT_WINDOW_DISPLAY_CHANGED,
                // Window display scale has been changed
                DisplayScaleChanged = SDL3::SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED,
                // The window has been occluded
                Occluded = SDL3::SDL_EVENT_WINDOW_OCCLUDED,
                // The window with the associated ID is being or has been destroyed.
                // If this message is being handled in an event watcher, the window
                // handle is still valid and can still be used to retrieve any userdata
                // associated with the window. Otherwise, the handle has already been
                // destroyed and all resources associated with it are invalid
                Destroyed = SDL3::SDL_EVENT_WINDOW_DESTROYED,
                // Window has gained focus of the pressure-sensitive pen with ID "data1"
                PenEnter = SDL3::SDL_EVENT_WINDOW_PEN_ENTER,
                // Window has lost focus of the pressure-sensitive pen with ID "data1"
                PenLeave = SDL3::SDL_EVENT_WINDOW_PEN_LEAVE,
                // The last Window::Event::ID possible (highest integer value)
                WindowLast = SDL3::SDL_EVENT_WINDOW_LAST,
            };
        };

        struct DisplayEvent
        {
            using Data = SDL3::SDL_DisplayEvent;

            enum ID : std::underlying_type_t<SDL3::SDL_EventType> {
                // The first Window::DisplayEvent::ID possible (lowest integer value)
                DisplayFirst = SDL3::SDL_EVENT_DISPLAY_FIRST,
                // Display orientation has changed to data1
                Orientation = SDL3::SDL_EVENT_DISPLAY_ORIENTATION,
                // Display has been added to the system
                Added = SDL3::SDL_EVENT_DISPLAY_ADDED,
                // Display has been removed from the system
                Removed = SDL3::SDL_EVENT_DISPLAY_REMOVED,
                // Display has changed position
                Moved = SDL3::SDL_EVENT_DISPLAY_MOVED,
                // Display has changed content scale
                ContentScaleChanged = SDL3::SDL_EVENT_DISPLAY_CONTENT_SCALE_CHANGED,
                // The last Window::DisplayEvent::ID possible (highest integer value)
                DisplayLast = SDL3::SDL_EVENT_DISPLAY_LAST,
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

        struct OpenGL
        {
            using sdl_type = SDL3::SDL_GLattr;

            enum Attribute : std::underlying_type_t<sdl_type> {
                RedSize = SDL3::SDL_GL_RED_SIZE,
                GreenSize = SDL3::SDL_GL_GREEN_SIZE,
                BlueSize = SDL3::SDL_GL_BLUE_SIZE,
                AlphaSize = SDL3::SDL_GL_ALPHA_SIZE,
                BufferSize = SDL3::SDL_GL_BUFFER_SIZE,
                Doublebuffer = SDL3::SDL_GL_DOUBLEBUFFER,
                DepthSize = SDL3::SDL_GL_DEPTH_SIZE,
                StencilSize = SDL3::SDL_GL_STENCIL_SIZE,
                AccumRedSize = SDL3::SDL_GL_ACCUM_RED_SIZE,
                AccumGreenSize = SDL3::SDL_GL_ACCUM_GREEN_SIZE,
                AccumBlueSize = SDL3::SDL_GL_ACCUM_BLUE_SIZE,
                AccumAlphaSize = SDL3::SDL_GL_ACCUM_ALPHA_SIZE,
                Stereo = SDL3::SDL_GL_STEREO,
                Multisamplebuffers = SDL3::SDL_GL_MULTISAMPLEBUFFERS,
                Multisamplesamples = SDL3::SDL_GL_MULTISAMPLESAMPLES,
                AcceleratedVisual = (sdl_type)SDL3::SDL_GL_ACCELERATED_VISUAL,
                RetainedBacking = SDL3::SDL_GL_RETAINED_BACKING,
                ContextMajorVersion = SDL3::SDL_GL_CONTEXT_MAJOR_VERSION,
                ContextMinorVersion = SDL3::SDL_GL_CONTEXT_MINOR_VERSION,
                ContextFlags = SDL3::SDL_GL_CONTEXT_FLAGS,
                ContextProfileMask = SDL3::SDL_GL_CONTEXT_PROFILE_MASK,
                ShareWithCurrentContext = SDL3::SDL_GL_SHARE_WITH_CURRENT_CONTEXT,
                FramebufferSrgbCapable = SDL3::SDL_GL_FRAMEBUFFER_SRGB_CAPABLE,
                ContextReleaseBehavior = SDL3::SDL_GL_CONTEXT_RELEASE_BEHAVIOR,
                ContextResetNotification = SDL3::SDL_GL_CONTEXT_RESET_NOTIFICATION,
                ContextNoError = SDL3::SDL_GL_CONTEXT_NO_ERROR,
                Floatbuffers = SDL3::SDL_GL_FLOATBUFFERS,
                EGLPlatform = SDL3::SDL_GL_EGL_PLATFORM
            };

            enum ContextFlag : std::underlying_type_t<SDL3::SDL_GLcontextFlag> {
                Debug = SDL3::SDL_GL_CONTEXT_DEBUG_FLAG,
                ForwardCompatible = SDL3::SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG,
                RobustAccess = SDL3::SDL_GL_CONTEXT_ROBUST_ACCESS_FLAG,
                ResetIsolation = SDL3::SDL_GL_CONTEXT_RESET_ISOLATION_FLAG,
            };

            enum Profile : std::underlying_type_t<SDL3::SDL_GLprofile> {
                Core = SDL3::SDL_GL_CONTEXT_PROFILE_CORE,
                Compatibility = SDL3::SDL_GL_CONTEXT_PROFILE_COMPATIBILITY,
                ES = SDL3::SDL_GL_CONTEXT_PROFILE_ES,
            };
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
        bool clear();
        bool redraw();

        bool draw_setup();
        bool draw_contents();
        bool draw_widgets();
        bool draw_teardown();
        bool draw_all();

        bool init_gui();
        bool set_vsync(bool enabled);
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
        bool set_opengl_attribute(OpenGL::Attribute attr, auto val);

        bool get_grab() const;
        f32 get_opacity() const;
        std::string get_title();
        DisplayID get_display();
        SDL3::SDL_WindowFlags get_flags() const;
        SDL3::SDL_DisplayMode get_display_mode() const;
        ds::dims<i32> get_size();
        ds::dims<i32> get_render_size();
        ds::dims<i32> get_min_size() const;
        ds::dims<i32> get_max_size() const;
        ds::point<i32> get_position() const;

        const std::unique_ptr<Renderer>& renderer() const;
        const rl::Window& operator=(rl::Window&& other) noexcept;
        SDL3::SDL_Window* sdl_handle() const;
        bool is_valid() const;
        bool swap_buffers();

    public:
        virtual ds::dims<i32> preferred_size(NVGcontext* nvg_context) const
        {
            return ds::dims<i32>{ 0, 0 };
        }

        virtual void perform_layout(NVGcontext* nvg_context)
        {
        }

        void update_focus(ui::widget* widget)
        {
        }

        void move_window_to_front(rl::Window* window)
        {
        }

        const std::string& title() const
        {
            return m_title;
        }

        void set_title(const std::string& title)
        {
            m_title = title;
        }

        bool modal() const
        {
            return m_modal;
        }

        void set_modal(bool modal)
        {
            m_modal = modal;
        }

        ui::widget* button_panel()
        {
        }

        void dispose_window(rl::Window* window)
        {
            if (std::find(m_focus_path.begin(), m_focus_path.end(), window) != m_focus_path.end())
                m_focus_path.clear();
            if (m_drag_widget == window)
                m_drag_widget = nullptr;

            this->remove_child(window);
        }

        void dispose()
        {
            ui::widget* owner{ this };
            while (owner->parent() != nullptr)
                owner = owner->parent();

            rl::Window* window{ static_cast<rl::Window*>(owner) };
            runtime_assert(window != nullptr, "Failed widget->window cast");
            runtime_assert(window != this, "Failed dynamic cast");
            window->dispose_window(this);
        }

        void center_window(rl::Window* window) const
        {
            if (window->size() == ds::dims<i32>{ 0, 0 })
            {
                auto&& pref_size{ window->preferred_size(m_nvg_context) };
                window->set_size(pref_size);
                window->perform_layout(m_nvg_context);
            }

            auto&& offset{ m_size - window->size() };
            window->set_position({
                offset.width / 2,
                offset.height / 2,
            });
        }

        void center()
        {
            ui::widget* owner{ this };
            while (owner->parent() != nullptr)
                owner = owner->parent();

            rl::Window* window{ static_cast<rl::Window*>(owner) };
            runtime_assert(window != nullptr, "Failed widget->window cast");
            runtime_assert(window != this, "window owns itself");
            window->center_window(this);
        }

    protected:
        friend class EventHandler;

        bool on_shown(const WindowID id);
        bool on_hidden(const WindowID id);
        bool on_exposed(const WindowID id);
        bool on_moved(const WindowID id, ds::point<i32>&& pt);
        bool on_resized(const WindowID id, ds::dims<i32>&& size);
        bool on_pixel_size_changed(const WindowID id, ds::dims<i32>&& pixel_size);
        bool on_minimized(const WindowID id);
        bool on_maximized(const WindowID id);
        bool on_restored(const WindowID id);
        bool on_mouse_enter(const WindowID id);
        bool on_mouse_leave(const WindowID id);
        bool on_mouse_click(const Mouse::Button::type button);
        bool on_mouse_move(const WindowID id, ds::point<i32>&& pos);
        bool on_mouse_drag(const WindowID id);
        bool on_mouse_scroll(Mouse::Event::Data::Wheel& wheel);
        bool on_kb_focus_gained(const WindowID id);
        bool on_kb_focus_lost(const WindowID id);
        bool on_close_requested(const WindowID id);
        bool on_take_focus(const WindowID id);
        bool on_hit_test(const WindowID id);
        bool on_icc_profile_changed(const WindowID id);
        bool on_display_changed(const WindowID id);
        bool on_display_scale_changed(const WindowID id);
        bool on_display_content_scale_changed(const DisplayID id);
        bool on_occluded(const WindowID id);
        bool on_destroyed(const WindowID id);

    private:
        std::string m_title{};
        ui::widget* m_drag_widget{ nullptr };
        std::vector<ui::widget*> m_focus_path;
        ds::point<i32> m_mouse_pos{ 0, 0 };
        std::function<void(ds::dims<i32>)> m_resize_callback;
        ds::color<u8> m_background_color{ 29, 32, 39 };
        rl::Timer<float> m_timer{};

        i32 m_mouse_state{ 0 };
        i32 m_modifiers{ 0 };

        bool m_vsync = false;
        bool m_modal{ false };
        bool m_depth_buffer{ false };
        bool m_stencil_buffer{ false };
        bool m_float_buffer{ false };
        bool m_drag_active{ false };
        bool m_process_events{ true };
        bool m_redraw{ true };

        // time in seconds recording the last interaction
        // with the current window widget, represented in
        // elapsed time since the window was created.
        float m_last_interaction{ 0.0f };

        /// <summary>
        ///   The content display scale relative to a window's pixel size.
        ///   <para>
        ///     This is a combination of the window pixel density and the display content scale,
        ///     and is the expected scale for displaying content in this window. For example, if a
        ///     3840x2160 window had a display scale of 2.0, the user expects the content to take
        ///     twice as many pixels and be the same physical size as if it were being displayed in
        ///     a 1920x1080 window with a display scale of 1.0. Conceptually this value corresponds
        ///     to the scale display setting, and is updated when that setting is changed, or the
        ///     window moves to a display with a different scale setting.
        ///   </para>
        /// </summary>
        float m_pixel_ratio = 0.0f;

        /// <summary>
        ///   The pixel density of a window.
        ///   <para>
        ///     This is a ratio of pixel size to window size.
        ///   </para>
        ///   <para>
        ///     For example, if the window is 1920x1080 and it has a high density back buffer of
        ///     3840x2160 pixels, it would have a pixel density of 2.0.
        ///   </para>
        /// </summary>
        float m_pixel_density = 0.0f;

    private:
        WindowID m_window_id{ 0 };
        DisplayID m_display_id{ 0 };
        Properties m_properties{ Properties::Flag::None };
        SDL3::SDL_Window* m_sdl_window{ nullptr };
        ds::rect<i32> m_window_rect{ 0, 0, 0, 0 };
        std::unique_ptr<rl::Renderer> m_renderer;
    };
}
