#pragma once

#include <bitset>
#include <memory>
#include <string>

#include "core/keyboard.hpp"
#include "core/mouse.hpp"
#include "core/ui/widget.hpp"
#include "ds/dims.hpp"
#include "ds/point.hpp"
#include "ds/rect.hpp"
#include "ds/vector2d.hpp"
#include "sdl/defs.hpp"

namespace rl {
    class EventHandler;
    class OpenGLRenderer;
    class NVGRenderer;

    using WindowID = SDL3::SDL_WindowID;
    using DisplayID = SDL3::SDL_DisplayID;

    namespace ui {
        class Canvas;
    }

    class MainWindow final
    {
        friend class Popup;
        friend class EventHandler;

    public:
        // clang-format off

        struct Event
        {
            using Data = SDL3::SDL_WindowEvent;

            enum ID : std::underlying_type_t<SDL3::SDL_EventType> {
                WindowFirst         = SDL3::SDL_EVENT_WINDOW_FIRST,                  // The first MainWindow::Event::ID possible (lowest integer value)
                Shown               = SDL3::SDL_EVENT_WINDOW_SHOWN,                  // MainWindow has been shown
                Hidden              = SDL3::SDL_EVENT_WINDOW_HIDDEN,                 // MainWindow has been hidden
                Exposed             = SDL3::SDL_EVENT_WINDOW_EXPOSED,                // MainWindow has been exposed and should be redrawn
                Moved               = SDL3::SDL_EVENT_WINDOW_MOVED,                  // The window has been moved to point<i32>{x=data1, y=data2}
                Resized             = SDL3::SDL_EVENT_WINDOW_RESIZED,                // The window has been resized to dims<i32>{w:data1, h:data2}
                PixelSizeChanged    = SDL3::SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED,     // The pixel size of the window has changed to dims<i32>{w:data1, h:data2}
                Minimized           = SDL3::SDL_EVENT_WINDOW_MINIMIZED,              // MainWindow has been minimized
                Maximized           = SDL3::SDL_EVENT_WINDOW_MAXIMIZED,              // MainWindow has been maximized
                Restored            = SDL3::SDL_EVENT_WINDOW_RESTORED,               // MainWindow has been restored to normal size and position
                MouseEnter          = SDL3::SDL_EVENT_WINDOW_MOUSE_ENTER,            // MainWindow has gained mouse focus
                MouseLeave          = SDL3::SDL_EVENT_WINDOW_MOUSE_LEAVE,            // MainWindow has lost mouse focus
                FocusGained         = SDL3::SDL_EVENT_WINDOW_FOCUS_GAINED,           // MainWindow has gained keyboard focus
                FocusLost           = SDL3::SDL_EVENT_WINDOW_FOCUS_LOST,             // MainWindow has lost keyboard focus
                CloseRequested      = SDL3::SDL_EVENT_WINDOW_CLOSE_REQUESTED,        // The window manager requests that the window be closed
                TakeFocus           = SDL3::SDL_EVENT_WINDOW_TAKE_FOCUS,             // MainWindow is being offered a focus.  Should SetWindowInputFocus() on itself or a subwindow, or ignore
                HitTest             = SDL3::SDL_EVENT_WINDOW_HIT_TEST,               // MainWindow had a hit test that wasn't SDL_HITTEST_NORMAL
                ICCProfChanged      = SDL3::SDL_EVENT_WINDOW_ICCPROF_CHANGED,        // The ICC profile of the window's display has changed
                DisplayChanged      = SDL3::SDL_EVENT_WINDOW_DISPLAY_CHANGED,        // MainWindow has been moved to display: data1
                DisplayScaleChanged = SDL3::SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED,  // MainWindow display scale has been changed
                Occluded            = SDL3::SDL_EVENT_WINDOW_OCCLUDED,               // The window has been occluded
                Destroyed           = SDL3::SDL_EVENT_WINDOW_DESTROYED,              // The window with the associated ID is being or has been destroyed.
                PenEnter            = SDL3::SDL_EVENT_WINDOW_PEN_ENTER,              // MainWindow has gained focus of the pressure-sensitive pen with ID "data1"
                PenLeave            = SDL3::SDL_EVENT_WINDOW_PEN_LEAVE,              // MainWindow has lost focus of the pressure-sensitive pen with ID "data1"
                WindowLast          = SDL3::SDL_EVENT_WINDOW_LAST,                   // The last MainWindow::Event::ID possible (highest integer value)
            };
        };

        struct DisplayEvent
        {
            using Data = SDL3::SDL_DisplayEvent;

            enum ID : std::underlying_type_t<SDL3::SDL_EventType> {
                DisplayFirst        = SDL3::SDL_EVENT_DISPLAY_FIRST,                 // The first MainWindow::DisplayEvent::ID possible (lowest integer value)
                Orientation         = SDL3::SDL_EVENT_DISPLAY_ORIENTATION,           // Display orientation has changed to data1
                Added               = SDL3::SDL_EVENT_DISPLAY_ADDED,                 // Display has been added to the system
                Removed             = SDL3::SDL_EVENT_DISPLAY_REMOVED,               // Display has been removed from the system
                Moved               = SDL3::SDL_EVENT_DISPLAY_MOVED,                 // Display has changed position
                ContentScaleChanged = SDL3::SDL_EVENT_DISPLAY_CONTENT_SCALE_CHANGED, // Display has changed content scale
                DisplayLast         = SDL3::SDL_EVENT_DISPLAY_LAST,                  // The last MainWindow::DisplayEvent::ID possible (highest integer value)
            }; 
        };

        struct Properties : std::bitset<sizeof(u32) * 8>
        {
            enum Flags : u32 {
                None            = 0,
                Fullscreen      = SDL_WINDOW_FULLSCREEN,          // window is in fullscreen mode
                OpenGL          = SDL_WINDOW_OPENGL,              // window usable with OpenGL context
                Occluded        = SDL_WINDOW_OCCLUDED,            // window is occluded
                Hidden          = SDL_WINDOW_HIDDEN,              // window is neither mapped onto the desktop nor shown in the taskbar/dock/window list; SDL_ShowWindow() is required for it to become visible
                Borderless      = SDL_WINDOW_BORDERLESS,          // no window decoration
                Resizable       = SDL_WINDOW_RESIZABLE,           // window can be resized
                Minimized       = SDL_WINDOW_MINIMIZED,           // window is minimized
                Maximized       = SDL_WINDOW_MAXIMIZED,           // window is maximized
                MouseGrabbed    = SDL_WINDOW_MOUSE_GRABBED,       // window has grabbed mouse input
                InputFocus      = SDL_WINDOW_INPUT_FOCUS,         // window has input focus
                MouseFocus      = SDL_WINDOW_MOUSE_FOCUS,         // window has mouse focus
                External        = SDL_WINDOW_EXTERNAL,            // window not created by SDL
                HighDPI         = SDL_WINDOW_HIGH_PIXEL_DENSITY,  // window uses high pixel density back buffer if possible
                MouseCapture    = SDL_WINDOW_MOUSE_CAPTURE,       // window has mouse captured (unrelated to MOUSE_GRABBED)
                AlwaysOnTop     = SDL_WINDOW_ALWAYS_ON_TOP,       // window should always be above others
                Utility         = SDL_WINDOW_UTILITY,             // window should be treated as a utility window, not showing in the task bar and window list
                Tooltip         = SDL_WINDOW_TOOLTIP,             // window should be treated as a tooltip
                PopupMenu       = SDL_WINDOW_POPUP_MENU,          // window should be treated as a popup menu
                KeyboardGrabbed = SDL_WINDOW_KEYBOARD_GRABBED,    // window has grabbed keyboard input
                Vulkan          = SDL_WINDOW_VULKAN,              // window usable for Vulkan surface
                Metal           = SDL_WINDOW_METAL,               // window usable for Metal view
                Transparent     = SDL_WINDOW_TRANSPARENT,         // window with transparent buffer
                NotFocusable    = SDL_WINDOW_NOT_FOCUSABLE,       // window should not be focusable
            };

            constexpr operator u32() const
            {
                return static_cast<u32>(this->to_ulong());
            }
        };

        struct OpenGL
        {
            enum Attribute : std::underlying_type_t<SDL3::SDL_GLattr> {
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

        constexpr static inline MainWindow::Properties DEFAULT_PROPERTY_FLAGS = {
            Properties::Flags::Resizable | Properties::Flags::OpenGL
        };

        constexpr static inline ds::point<i32> DEFAULT_POSITION{
            SDL_WINDOWPOS_CENTERED_MASK,
            SDL_WINDOWPOS_CENTERED_MASK,
        };

        constexpr static inline ds::dims<i32> DEFAULT_SIZE = {
            1920,
            1080,
        };

    public:
        explicit MainWindow(const std::string& title, const ds::dims<i32>& dims = DEFAULT_SIZE,
                            Properties flags = DEFAULT_PROPERTY_FLAGS);

        ~MainWindow();

        MainWindow& operator=(MainWindow&& other) noexcept;
        MainWindow& operator=(const MainWindow& other) = delete;

    public:
        bool raise() const;
        bool restore() const;
        bool minimize() const;
        bool maximize() const;
        bool hide() const;
        bool show() const;

        bool clear() const;
        bool render_start() const;
        bool render() const;
        bool render_end() const;
        bool swap_buffers() const;

        std::string get_title();
        DisplayID get_display_id();
        WindowID window_id() const;
        WindowID get_window_id();
        const ds::dims<i32>& get_size();
        const ds::dims<i32>& get_render_size();

        bool is_valid() const;
        bool input_grabbed() const;
        ds::dims<i32> get_min_size() const;
        ds::dims<i32> get_max_size() const;
        ds::point<i32> get_position() const;
        f32 get_opacity() const;

        SDL3::SDL_Window* sdl_handle() const;
        MainWindow::Properties::Flags get_flags() const;
        SDL3::SDL_DisplayMode get_display_mode() const;

        ui::Canvas* gui() const;

        const std::unique_ptr<OpenGLRenderer>& glrenderer() const;
        const std::unique_ptr<NVGRenderer>& vgrenderer() const;

        const Keyboard& keyboard() const;
        const Mouse& mouse() const;

        bool set_vsync(bool enabled);
        bool set_grab(bool grabbed) const;
        bool set_bordered(bool bordered) const;
        bool set_resizable(bool resizable) const;
        bool set_fullscreen(bool fullscreen) const;
        bool set_opacity(f32 opacity) const;
        bool set_position(const ds::point<i32>& pos);
        bool set_size(const ds::dims<i32>& size);
        bool set_min_size(const ds::dims<i32>& size) const;
        bool set_max_size(const ds::dims<i32>& size) const;
        bool set_opengl_attribute(OpenGL::Attribute attr, auto val);
        bool set_background(const ds::color<u8>& background) const;
        bool set_title(const std::string& title);
        bool set_modal(bool modal) const;

    protected:
        void mouse_moved_event_callback(const SDL3::SDL_Event& e);
        void mouse_wheel_event_callback(const SDL3::SDL_Event& e);
        void mouse_button_pressed_event_callback(const SDL3::SDL_Event& e);
        void mouse_button_released_event_callback(const SDL3::SDL_Event& e);
        void mouse_entered_event_callback(const SDL3::SDL_Event& e);
        void mouse_exited_event_callback(const SDL3::SDL_Event& e);
        void keyboard_key_pressed_event_callback(const SDL3::SDL_Event& e);
        void keyboard_key_released_event_callback(const SDL3::SDL_Event& e);
        void keyboard_char_event_callback(const SDL3::SDL_Event& e);
        void window_focus_gained_event_callback(const SDL3::SDL_Event& e) const;
        void window_focus_lost_event_callback(const SDL3::SDL_Event& e) const;
        void window_resized_event_callback(const SDL3::SDL_Event& e);
        void window_moved_event_callback(const SDL3::SDL_Event& e);

    protected:
        bool window_shown_event_callback(const SDL3::SDL_Event& e) const;
        bool window_occluded_event_callback(const SDL3::SDL_Event& e) const;
        bool window_hidden_event_callback(const SDL3::SDL_Event& e) const;
        bool window_exposed_event_callback(const SDL3::SDL_Event& e) const;
        bool window_minimized_event_callback(const SDL3::SDL_Event& e) const;
        bool window_maximized_event_callback(const SDL3::SDL_Event& e) const;
        bool window_restored_event_callback(const SDL3::SDL_Event& e) const;
        bool window_close_requested_event_callback(const SDL3::SDL_Event& e) const;
        bool window_take_focus_event_callback(const SDL3::SDL_Event& e) const;
        bool window_hit_test_event_callback(const SDL3::SDL_Event& e) const;
        bool window_icc_profile_changed_callback(const SDL3::SDL_Event& e) const;
        bool window_pixel_size_changed_event_callback(const SDL3::SDL_Event& e);
        bool window_display_changed_event_callback(const SDL3::SDL_Event& e) const;
        bool window_display_scale_changed_event_callback(const SDL3::SDL_Event& e) const;
        bool window_destroyed_event_callback(const SDL3::SDL_Event& e) const;

    public:
        explicit MainWindow(const MainWindow& window) = delete;
        explicit MainWindow(MainWindow&& window) noexcept = delete;
        explicit MainWindow(SDL3::SDL_Window* other) = delete;

        // TODO: remove
        static std::string name()
        {
            return "MainWindow;";
        }

    private:
        SDL3::SDL_Window* m_sdl_window{ nullptr };
        std::unique_ptr<NVGRenderer> m_vg_renderer;
        std::unique_ptr<OpenGLRenderer> m_gl_renderer;
        Keyboard m_keyboard{};
        Mouse m_mouse{};

        std::string m_title{};
        WindowID m_window_id{ 0 };
        DisplayID m_display_id{ 0 };
        Properties m_properties{ Properties::None };
        ds::rect<i32> m_window_rect{ 0, 0, 0, 0 };
        ui::Canvas* m_gui_canvas{ nullptr };
        ds::dims<i32> m_framebuf_size{ 0, 0 };

        f32 m_pixel_ratio{ 1.0f };
        f32 m_pixel_density{ 1.0f };
        bool m_vsync{ true };
    };
}
