#include <iostream>
#include <map>

#include "core/assert.hpp"
#include "gui/popup.hpp"
#include "gui/screen.hpp"
#include "gui/theme.hpp"
#include "gui/window.hpp"
#include "sdl/defs.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL.h>
SDL_C_LIB_END

namespace rl::gui {
    Screen::Screen()
        : gui::Widget{ nullptr }
        , m_sdl_window{ nullptr }
        , m_sdl_renderer{ nullptr }
        , m_mouse_state{ 0 }
        , m_modifiers{ 0 }
        , m_mouse_pos{ 0, 0 }
        , m_drag_active{ false }
        , m_last_interaction{ SDL3::SDL_GetTicks() }
        , m_process_events{ true }
        , m_background{ Color(0.3f, 0.3f, 0.32f, 1.0f) }
    {
        m_visible = true;
        m_theme = new gui::Theme(m_sdl_renderer);
    }

    Screen::Screen(SDL3::SDL_Window* window, const Vector2i& size, const std::string& caption,
                   bool resizable, bool fullscreen)
        : Widget(nullptr)
        , m_sdl_window(nullptr)
        , m_sdl_renderer(nullptr)
        , m_caption(caption)
    {
        SDL_SetWindowTitle(window, caption.c_str());
        init(window);
    }

    bool Screen::on_event(SDL3::SDL_Event& event)
    {
        switch (event.type)
        {
            case SDL3::SDL_EVENT_MOUSE_WHEEL:
            {
                if (!m_process_events)
                    return false;
                return scroll_event_callback(event.wheel.x, event.wheel.y);
            }
            break;

            case SDL3::SDL_EVENT_MOUSE_MOTION:
            {
                if (!m_process_events)
                    return false;
                return cursor_pos_event_callback(event.motion.x, event.motion.y);
            }
            break;

            case SDL3::SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL3::SDL_EVENT_MOUSE_BUTTON_UP:
            {
                if (!m_process_events)
                    return false;

                SDL3::SDL_Keymod mods = SDL3::SDL_GetModState();
                return mouse_button_event_callback(event.button.button, event.button.type, mods);
            }
            break;

            case SDL3::SDL_EVENT_KEY_DOWN:
            case SDL3::SDL_EVENT_KEY_UP:
            {
                if (!m_process_events)
                    return false;

                SDL3::SDL_Keymod mods = SDL3::SDL_GetModState();
                return keyboard_event_callback(event.key.keysym.sym, event.key.keysym.scancode,
                                               event.key.state, mods);
            }
            break;

            case SDL3::SDL_EVENT_TEXT_INPUT:
            {
                if (!m_process_events)
                    return false;
                return character_event_callback(event.text.text[0]);
            }
            break;
        }
        return false;
    }

    void Screen::init(SDL3::SDL_Window* window)
    {
        m_sdl_window = window;
        SDL3::SDL_GetWindowSize(window, &m_size[0], &m_size[1]);
        SDL3::SDL_GetWindowSize(window, &m_framebuf_size[0], &m_framebuf_size[1]);

        SDL3::SDL_Surface* surface = SDL3::SDL_CreateSurface(m_size.x, m_size.y,
                                                             SDL3::SDL_PIXELFORMAT_RGBA8888);
        m_sdl_renderer = SDL3::SDL_GetRenderer(window);
        if (m_sdl_renderer == nullptr)
        {
            // SDL3::SDL_GetWindowSurface(window);
            m_sdl_renderer = SDL3::SDL_CreateRenderer(window, "opengl", SDL3::SDL_WINDOW_OPENGL);
        }

        runtime_assert(m_sdl_renderer != nullptr, "Failed to init gui renderer");

        m_visible = true;
        m_theme = new Theme(m_sdl_renderer);
        m_mouse_pos = { 0, 0 };
        m_mouse_state = m_modifiers = 0;
        m_drag_active = false;
        m_last_interaction = SDL3::SDL_GetTicks();
        m_process_events = true;
        m_background = Color(0.3f, 0.3f, 0.32f, 1.0f);
    }

    Screen::~Screen()
    {
    }

    void Screen::set_visible(bool visible)
    {
        if (m_visible != visible)
        {
            m_visible = visible;

            if (visible)
                SDL3::SDL_ShowWindow(m_sdl_window);
            else
                SDL3::SDL_HideWindow(m_sdl_window);
        }
    }

    void Screen::set_caption(const std::string& caption)
    {
        if (caption != m_caption)
        {
            SDL3::SDL_SetWindowTitle(m_sdl_window, caption.c_str());
            m_caption = caption;
        }
    }

    void Screen::set_size(const Vector2i& size)
    {
        Widget::set_size(size);
        SDL3::SDL_SetWindowSize(m_sdl_window, size.x, size.y);
    }

    void Screen::draw_all()
    {
        this->draw_contents();
        this->draw_gui();
    }

    void Screen::draw_gui()
    {
        if (!m_visible)
            return;

        // Calculate pixel ratio for hi-dpi devices.
        m_pixel_ratio = (float)m_framebuf_size[0] / (float)m_size[0];

        SDL3::SDL_Renderer* renderer = SDL3::SDL_GetRenderer(m_sdl_window);
        Widget::draw(renderer);

        float elapsed_sec = float(SDL3::SDL_GetTicks() - m_last_interaction) /
                            float(SDL_MS_PER_SECOND);
        if (elapsed_sec > 0.5f)
        {
            /* Draw tooltips */
            const Widget* widget = findWidget(m_mouse_pos);
            if (widget && !widget->tooltip().empty())
            {
                int tooltipWidth = 150;

                if (m_last_tooltip != widget->tooltip())
                {
                    m_last_tooltip = widget->tooltip();
                    m_theme->getTexAndRectUtf8(renderer, m_tooltip_texture, 0, 0,
                                               m_last_tooltip.c_str(), "sans", 15, Color(1.f, 1.f));
                }

                if (m_tooltip_texture.tex)
                {
                    Vector2i pos = widget->absolute_position() +
                                   Vector2i(widget->width() / 2, widget->height() + 10);

                    float alpha = (std::min(1.0f, 2.0f * (elapsed_sec - 0.5f)) * 0.8f) * 255.0f;
                    SDL3::SDL_SetTextureAlphaMod(m_tooltip_texture.tex, uint8_t(alpha));

                    SDL3::SDL_FRect bgrect{
                        static_cast<float>(pos.x - 2),
                        static_cast<float>(pos.y - 2 - m_tooltip_texture.h()),
                        static_cast<float>(m_tooltip_texture.w() + 4),
                        static_cast<float>(m_tooltip_texture.h() + 4),
                    };

                    SDL3::SDL_FRect ttrect{ static_cast<float>(pos.x),
                                            static_cast<float>(pos.y - m_tooltip_texture.h()), 0,
                                            0 };
                    SDL3::SDL_SetRenderDrawColor(renderer, 0, 0, 0, static_cast<uint8_t>(alpha));
                    SDL3::SDL_RenderFillRect(renderer, &bgrect);
                    SDL3::SDL_RenderTexture(renderer, m_tooltip_texture.tex, &ttrect, nullptr);
                    SDL3::SDL_SetRenderDrawColor(renderer, 255, 255, 255,
                                                 static_cast<uint8_t>(alpha));
                    SDL3::SDL_RenderLine(renderer, bgrect.x, bgrect.y, bgrect.x + bgrect.w,
                                         bgrect.y);
                    SDL3::SDL_RenderLine(renderer, bgrect.x + bgrect.w, bgrect.y,
                                         bgrect.x + bgrect.w, bgrect.y + bgrect.h);
                    SDL3::SDL_RenderLine(renderer, bgrect.x, bgrect.y + bgrect.h,
                                         bgrect.x + bgrect.w, bgrect.y + bgrect.h);
                    SDL3::SDL_RenderLine(renderer, bgrect.x, bgrect.y, bgrect.x,
                                         bgrect.y + bgrect.h);
                }
            }
        }
    }

    bool Screen::kb_button_event(int key, int scancode, int action, int modifiers)
    {
        if (mFocusPath.size() > 0)
        {
            for (auto it = mFocusPath.rbegin() + 1; it != mFocusPath.rend(); ++it)
                if ((*it)->focused() && (*it)->kb_button_event(key, scancode, action, modifiers))
                    return true;
        }

        return false;
    }

    bool Screen::kb_character_event(unsigned int codepoint)
    {
        if (mFocusPath.size() > 0)
        {
            for (auto it = mFocusPath.rbegin() + 1; it != mFocusPath.rend(); ++it)
                if ((*it)->focused() && (*it)->kb_character_event(codepoint))
                    return true;
        }
        return false;
    }

    bool Screen::cursor_pos_event_callback(double x, double y)
    {
        Vector2i p((int)x, (int)y);
        bool ret = false;
        m_last_interaction = SDL3::SDL_GetTicks();
        try
        {
            p -= Vector2i(1, 2);

            if (!m_drag_active)
            {
                Widget* widget = findWidget(p);
                /*if (widget != nullptr && widget->cursor() != mCursor) {
                    mCursor = widget->cursor();
                    glfwSetCursor(mGLFWWindow, mCursors[(int) mCursor]);
                }*/
            }
            else
            {
                ret = m_drag_widget->mouseDragEvent(p - m_drag_widget->parent()->absolute_position(),
                                                    p - m_mouse_pos, m_mouse_state, m_modifiers);
            }

            if (!ret)
                ret = mouseMotionEvent(p, p - m_mouse_pos, m_mouse_state, m_modifiers);

            m_mouse_pos = p;

            return ret;
        }
        catch (const std::exception& e)
        {
            std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
            abort();
        }
    }

    bool Screen::mouse_button_event_callback(int button, int action, int modifiers)
    {
        m_modifiers = modifiers;
        m_last_interaction = SDL3::SDL_GetTicks();
        try
        {
            if (mFocusPath.size() > 1)
            {
                const Window* window = dynamic_cast<Window*>(mFocusPath[mFocusPath.size() - 2]);
                if (window && window->modal())
                {
                    if (!window->contains(m_mouse_pos))
                        return false;
                }
            }

            if (action == SDL3::SDL_EVENT_MOUSE_BUTTON_DOWN)
                m_mouse_state |= 1 << button;
            else
                m_mouse_state &= ~(1 << button);

            auto dropWidget = findWidget(m_mouse_pos);
            if (m_drag_active && action == SDL3::SDL_EVENT_MOUSE_BUTTON_UP &&
                dropWidget != m_drag_widget)
                m_drag_widget->mouseButtonEvent(
                    m_mouse_pos - m_drag_widget->parent()->absolute_position(), button, false,
                    m_modifiers);

            /*if (dropWidget != nullptr && dropWidget->cursor() != mCursor) {
                mCursor = dropWidget->cursor();
                glfwSetCursor(mGLFWWindow, mCursors[(int) mCursor]);
            }*/

            if (action == SDL3::SDL_EVENT_MOUSE_BUTTON_DOWN && button == SDL_BUTTON_LEFT)
            {
                m_drag_widget = findWidget(m_mouse_pos);
                if (m_drag_widget == this)
                    m_drag_widget = nullptr;
                m_drag_active = m_drag_widget != nullptr;
                if (!m_drag_active)
                    update_focus(nullptr);
            }
            else
            {
                m_drag_active = false;
                m_drag_widget = nullptr;
            }

            return mouseButtonEvent(m_mouse_pos, button,
                                    action == SDL3::SDL_EVENT_MOUSE_BUTTON_DOWN, m_modifiers);
        }
        catch (const std::exception& e)
        {
            std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
            abort();
        }
    }

    bool Screen::keyboard_event_callback(int key, int scancode, int action, int mods)
    {
        m_last_interaction = SDL3::SDL_GetTicks();
        try
        {
            return kb_button_event(key, scancode, action, mods);
        }
        catch (const std::exception& e)
        {
            std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
            abort();
        }
    }

    bool Screen::character_event_callback(unsigned int codepoint)
    {
        m_last_interaction = SDL3::SDL_GetTicks();
        try
        {
            return kb_character_event(codepoint);
        }
        catch (const std::exception& e)
        {
            std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
            abort();
        }
    }

    bool Screen::drop_event_callback(int count, const char** filenames)
    {
        std::vector<std::string> arg(count);
        for (int i = 0; i < count; ++i)
            arg[i] = filenames[i];
        return drop_event(arg);
    }

    bool Screen::scroll_event_callback(double x, double y)
    {
        m_last_interaction = SDL3::SDL_GetTicks();
        try
        {
            if (mFocusPath.size() > 1)
            {
                const Window* window = dynamic_cast<Window*>(mFocusPath[mFocusPath.size() - 2]);
                if (window && window->modal())
                {
                    if (!window->contains(m_mouse_pos))
                        return false;
                }
            }
            return scrollEvent(m_mouse_pos, Vector2f((float)x, (float)y));
        }
        catch (const std::exception& e)
        {
            std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
            abort();
        }
    }

    bool Screen::resize_event_callback(int, int)
    {
        Vector2i fbSize, size;
        // glfwGetFramebufferSize(mGLFWWindow, &fbSize[0], &fbSize[1]);
        SDL_GetWindowSize(m_sdl_window, &size[0], &size[1]);

        if (m_framebuf_size == Vector2i(0, 0) || size == Vector2i(0, 0))
            return false;

        m_framebuf_size = fbSize;
        m_size = size;
        m_last_interaction = SDL3::SDL_GetTicks();

        try
        {
            return resize_event(m_size);
        }
        catch (const std::exception& e)
        {
            std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
            abort();
        }
    }

    void Screen::update_focus(Widget* widget)
    {
        for (auto w : mFocusPath)
        {
            if (!w->focused())
                continue;
            w->focusEvent(false);
        }
        mFocusPath.clear();
        Widget* window = nullptr;
        while (widget)
        {
            mFocusPath.push_back(widget);
            if (dynamic_cast<Window*>(widget))
                window = widget;
            widget = widget->parent();
        }
        for (auto it = mFocusPath.rbegin(); it != mFocusPath.rend(); ++it)
            (*it)->focusEvent(true);

        if (window)
            move_window_to_front((Window*)window);
    }

    void Screen::dispose_window(Window* window)
    {
        if (std::find(mFocusPath.begin(), mFocusPath.end(), window) != mFocusPath.end())
            mFocusPath.clear();
        if (m_drag_widget == window)
            m_drag_widget = nullptr;
        remove_child(window);
    }

    void Screen::center_window(Window* window)
    {
        if (window->size() == Vector2i{ 0, 0 })
        {
            window->set_size(window->preferredSize(m_sdl_renderer));
            window->perform_layout(m_sdl_renderer);
        }
        window->set_relative_position((m_size - window->size()) / 2);
    }

    void Screen::move_window_to_front(Window* window)
    {
        m_children.erase(std::remove(m_children.begin(), m_children.end(), window),
                         m_children.end());
        m_children.push_back(window);
        /* Brute force topological sort (no problem for a few windows..) */
        bool changed = false;
        do
        {
            size_t baseIndex = 0;
            for (size_t index = 0; index < m_children.size(); ++index)
                if (m_children[index] == window)
                    baseIndex = index;
            changed = false;
            for (size_t index = 0; index < m_children.size(); ++index)
            {
                Popup* pw = dynamic_cast<Popup*>(m_children[index]);
                if (pw && pw->parentWindow() == window && index < baseIndex)
                {
                    move_window_to_front(pw);
                    changed = true;
                    break;
                }
            }
        }
        while (changed);
    }

    void Screen::perform_layout(SDL3::SDL_Renderer* ctx)
    {
        Widget::perform_layout(ctx);
    }

    void Screen::perform_layout()
    {
        Widget::perform_layout(m_sdl_renderer);
    }
}
