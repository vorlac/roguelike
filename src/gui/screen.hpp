#pragma once

#include "ds/dims.hpp"
#include "gui/window.hpp"
#include "sdl/defs.hpp"
#include "utils/numeric.hpp"

SDL_C_LIB_BEGIN
union SDL_Event;
struct SDL_Window;
SDL_C_LIB_END

namespace rl::gui {
    /**
     * @brief Represents a display surface (i.e. a full-screen or windowed GLFW window)
     * and forms the root element of a hierarchy of sdlgui widgets
     */
    class Screen : public Widget
    {
        friend class Widget;
        friend class Window;

    public:
        /**
         * @brief  Used to init empty window, use init(SDL_Window* window) for deferred init
         **/
        Screen();

        /**
         * @brief  Create a new screen
         **/
        Screen(SDL3::SDL_Window* window, const Vector2i& size, const std::string& caption,
               bool resizable = true, bool fullscreen = false);

        /**
         * @brief  Release all resources
         **/
        virtual ~Screen() override;

        /**
         * @brief  Get the window titlebar caption
         **/
        const std::string& caption() const
        {
            return m_caption;
        }

        /**
         * @brief  Set the window titlebar caption
         **/
        void set_caption(const std::string& caption);

        /**
         * @brief  Return the screen's background color
         **/
        const Color& background() const
        {
            return m_background;
        }

        /**
         * @brief  Set the screen's background color
         **/
        void set_background(const Color& background)
        {
            m_background = background;
        }

        /**
         * @brief  Set the top-level window visibility (no effect on full-screen windows)
         **/
        void set_visible(bool visible);

        /**
         * @brief  Set window size
         **/
        void set_size(const Vector2i& size);

        /**
         * @brief  Return the ratio between pixel and device coordinates (e.g. >= 2 on Mac Retina
         *displays)
         **/
        float pixel_ratio() const
        {
            return m_pixel_ratio;
        }

        virtual bool on_event(SDL3::SDL_Event& event);

        /**
         * @brief Draw the window contents
         * */
        virtual void draw_contents()
        {
            // override
            // TODO: OpenGL draw calls here
        }

        /**
         * @brief  Handle a file drop event
         **/
        virtual bool drop_event(const std::vector<std::string>& /* filenames */)
        {
            // override
            return false;
        }

        /**
         * @brief  Default keyboard event handler
         **/
        virtual bool kb_button_event(int key, int scancode, int action, int modifiers) override;

        /**
         * @brief  Text input event handler: codepoint is native endian UTF-32 format
         **/
        virtual bool kb_character_event(unsigned int codepoint) override;

        /**
         * @brief  Window resize event handler
         **/
        virtual bool resize_event(const Vector2i&)
        {
            return false;
        }

        virtual void draw_all();

        /**
         * @brief  Return the last observed mouse position value
         **/
        Vector2i mouse_pos() const
        {
            return m_mouse_pos;
        }

        /**
         * @brief  Return a pointer to the underlying GLFW window data structure
         **/
        SDL3::SDL_Window* window()
        {
            return m_sdl_window;
        }

        /**
         * @brief  Return a pointer to the underlying nanoVG draw context
         **/
        SDL3::SDL_Renderer* sdl_renderer()
        {
            return m_sdl_renderer;
        }

        /**
         * @brief  Compute the layout of all widgets
         **/
        void perform_layout();

        template <typename... TArgs>
        Window& window(const TArgs&... args)
        {
            return wdg<Window>(args...);
        }

    public:
        /**
         * @brief  Initialize the \ref Screen
         **/
        void init();
        void init(SDL3::SDL_Window* window);

        // Event handlers
        bool cursor_pos_event_callback(double x, double y);
        bool mouse_button_event_callback(int button, int action, int modifiers);
        bool keyboard_event_callback(int key, int scancode, int action, int mods);
        bool character_event_callback(unsigned int codepoint);
        bool drop_event_callback(int count, const char** filenames);
        bool scroll_event_callback(double x, double y);
        bool resize_event_callback(int width, int height);

        // Internal helper functions
        void update_focus(Widget* widget);
        void dispose_window(Window* window);
        void center_window(Window* window);
        void move_window_to_front(Window* window);
        void perform_layout(SDL3::SDL_Renderer* renderer) override;
        void draw_gui();

    protected:
        SDL3::SDL_Window* m_sdl_window{ nullptr };
        std::vector<gui::Widget*> m_focus_path{};
        SDL3::SDL_Renderer* m_sdl_renderer{ nullptr };
        ds::dims<i32> m_framebuf_size{ 0, 0 };
        float m_pixel_ratio{ 0.0f };
        int m_mouse_state{ 0 };
        int m_modifiers{ 0 };
        gui::Vector2i m_mouse_pos{ 0, 0 };
        bool m_drag_active{ false };
        gui::Widget* m_drag_widget{ nullptr };
        uint64_t m_last_interaction{ 0 };
        bool m_process_events{ false };
        gui::Color m_background{ 0, 0, 0, 0 };
        std::string m_caption{};
        std::string m_last_tooltip{};
        gui::Texture m_tooltip_texture{};
    };
}
