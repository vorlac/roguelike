#include <iostream>
#include <map>

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
        , mMouseState{ 0 }
        , mModifiers{ 0 }
        , mMousePos{ 0, 0 }
        , mDragActive{ false }
        , mLastInteraction{ SDL3::SDL_GetTicks() }
        , mProcessEvents{ true }
        , mBackground{ Color(0.3f, 0.3f, 0.32f, 1.0f) }
    {
        m_visible = true;
        m_theme = new gui::Theme(m_sdl_renderer);
    }

    Screen::Screen(SDL3::SDL_Window* window, const Vector2i& size, const std::string& caption,
                   bool resizable, bool fullscreen)
        : Widget(nullptr)
        , m_sdl_window(nullptr)
        , m_sdl_renderer(nullptr)
        , mCaption(caption)
    {
        SDL_SetWindowTitle(window, caption.c_str());
        initialize(window);
    }

    bool Screen::onEvent(SDL3::SDL_Event& event)
    {
        switch (event.type)
        {
            case SDL3::SDL_EVENT_MOUSE_WHEEL:
            {
                if (!mProcessEvents)
                    return false;
                return scrollCallbackEvent(event.wheel.x, event.wheel.y);
            }
            break;

            case SDL3::SDL_EVENT_MOUSE_MOTION:
            {
                if (!mProcessEvents)
                    return false;
                return cursorPosCallbackEvent(event.motion.x, event.motion.y);
            }
            break;

            case SDL3::SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL3::SDL_EVENT_MOUSE_BUTTON_UP:
            {
                if (!mProcessEvents)
                    return false;

                SDL3::SDL_Keymod mods = SDL3::SDL_GetModState();
                return mouseButtonCallbackEvent(event.button.button, event.button.type, mods);
            }
            break;

            case SDL3::SDL_EVENT_KEY_DOWN:
            case SDL3::SDL_EVENT_KEY_UP:
            {
                if (!mProcessEvents)
                    return false;

                SDL3::SDL_Keymod mods = SDL3::SDL_GetModState();
                return keyCallbackEvent(event.key.keysym.sym, event.key.keysym.scancode,
                                        event.key.state, mods);
            }
            break;

            case SDL3::SDL_EVENT_TEXT_INPUT:
            {
                if (!mProcessEvents)
                    return false;
                return charCallbackEvent(event.text.text[0]);
            }
            break;
        }
        return false;
    }

    void Screen::initialize(SDL3::SDL_Window* window)
    {
        m_sdl_window = window;
        SDL3::SDL_GetWindowSize(window, &mSize[0], &mSize[1]);
        SDL3::SDL_GetWindowSize(window, &mFBSize[0], &mFBSize[1]);
        m_sdl_renderer = SDL3::SDL_GetRenderer(window);

        if (m_sdl_renderer == nullptr)
            throw std::runtime_error("Could not initialize NanoVG!");

        m_visible = true;
        m_theme = new Theme(m_sdl_renderer);
        mMousePos = { 0, 0 };
        mMouseState = mModifiers = 0;
        mDragActive = false;
        mLastInteraction = SDL3::SDL_GetTicks();
        mProcessEvents = true;
        mBackground = Color(0.3f, 0.3f, 0.32f, 1.0f);
    }

    Screen::~Screen()
    {
    }

    void Screen::setVisible(bool visible)
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

    void Screen::setCaption(const std::string& caption)
    {
        if (caption != mCaption)
        {
            SDL3::SDL_SetWindowTitle(m_sdl_window, caption.c_str());
            mCaption = caption;
        }
    }

    void Screen::setSize(const Vector2i& size)
    {
        Widget::setSize(size);
        SDL3::SDL_SetWindowSize(m_sdl_window, size.x, size.y);
    }

    void Screen::drawAll()
    {
        drawContents();
        drawWidgets();
    }

    void Screen::drawWidgets()
    {
        if (!m_visible)
            return;

        /* Calculate pixel ratio for hi-dpi devices. */
        mPixelRatio = (float)mFBSize[0] / (float)mSize[0];

        SDL3::SDL_Renderer* renderer = SDL3::SDL_GetRenderer(m_sdl_window);
        draw(renderer);

        float elapsed_sec = float(SDL3::SDL_GetTicks() - mLastInteraction) /
                            float(SDL_MS_PER_SECOND);
        if (elapsed_sec > 0.5f)
        {
            /* Draw tooltips */
            const Widget* widget = findWidget(mMousePos);
            if (widget && !widget->tooltip().empty())
            {
                int tooltipWidth = 150;

                if (_lastTooltip != widget->tooltip())
                {
                    _lastTooltip = widget->tooltip();
                    m_theme->getTexAndRectUtf8(renderer, _tooltipTex, 0, 0, _lastTooltip.c_str(),
                                               "sans", 15, Color(1.f, 1.f));
                }

                if (_tooltipTex.tex)
                {
                    Vector2i pos = widget->absolutePosition() +
                                   Vector2i(widget->width() / 2, widget->height() + 10);

                    float alpha = (std::min(1.0f, 2.0f * (elapsed_sec - 0.5f)) * 0.8f) * 255.0f;
                    SDL3::SDL_SetTextureAlphaMod(_tooltipTex.tex, uint8_t(alpha));

                    SDL3::SDL_FRect bgrect{
                        static_cast<float>(pos.x - 2),
                        static_cast<float>(pos.y - 2 - _tooltipTex.h()),
                        static_cast<float>(_tooltipTex.w() + 4),
                        static_cast<float>(_tooltipTex.h() + 4),
                    };

                    SDL3::SDL_FRect ttrect{ static_cast<float>(pos.x),
                                            static_cast<float>(pos.y - _tooltipTex.h()), 0, 0 };
                    SDL3::SDL_SetRenderDrawColor(renderer, 0, 0, 0, static_cast<uint8_t>(alpha));
                    SDL3::SDL_RenderFillRect(renderer, &bgrect);
                    SDL3::SDL_RenderTexture(renderer, _tooltipTex.tex, &ttrect, nullptr);
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

    bool Screen::keyboardEvent(int key, int scancode, int action, int modifiers)
    {
        if (mFocusPath.size() > 0)
        {
            for (auto it = mFocusPath.rbegin() + 1; it != mFocusPath.rend(); ++it)
                if ((*it)->focused() && (*it)->keyboardEvent(key, scancode, action, modifiers))
                    return true;
        }

        return false;
    }

    bool Screen::keyboardCharacterEvent(unsigned int codepoint)
    {
        if (mFocusPath.size() > 0)
        {
            for (auto it = mFocusPath.rbegin() + 1; it != mFocusPath.rend(); ++it)
                if ((*it)->focused() && (*it)->keyboardCharacterEvent(codepoint))
                    return true;
        }
        return false;
    }

    bool Screen::cursorPosCallbackEvent(double x, double y)
    {
        Vector2i p((int)x, (int)y);
        bool ret = false;
        mLastInteraction = SDL3::SDL_GetTicks();
        try
        {
            p -= Vector2i(1, 2);

            if (!mDragActive)
            {
                Widget* widget = findWidget(p);
                /*if (widget != nullptr && widget->cursor() != mCursor) {
                    mCursor = widget->cursor();
                    glfwSetCursor(mGLFWWindow, mCursors[(int) mCursor]);
                }*/
            }
            else
            {
                ret = mDragWidget->mouseDragEvent(p - mDragWidget->parent()->absolutePosition(),
                                                  p - mMousePos, mMouseState, mModifiers);
            }

            if (!ret)
                ret = mouseMotionEvent(p, p - mMousePos, mMouseState, mModifiers);

            mMousePos = p;

            return ret;
        }
        catch (const std::exception& e)
        {
            std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
            abort();
        }
    }

    bool Screen::mouseButtonCallbackEvent(int button, int action, int modifiers)
    {
        mModifiers = modifiers;
        mLastInteraction = SDL3::SDL_GetTicks();
        try
        {
            if (mFocusPath.size() > 1)
            {
                const Window* window = dynamic_cast<Window*>(mFocusPath[mFocusPath.size() - 2]);
                if (window && window->modal())
                {
                    if (!window->contains(mMousePos))
                        return false;
                }
            }

            if (action == SDL3::SDL_EVENT_MOUSE_BUTTON_DOWN)
                mMouseState |= 1 << button;
            else
                mMouseState &= ~(1 << button);

            auto dropWidget = findWidget(mMousePos);
            if (mDragActive && action == SDL3::SDL_EVENT_MOUSE_BUTTON_UP &&
                dropWidget != mDragWidget)
                mDragWidget->mouseButtonEvent(mMousePos - mDragWidget->parent()->absolutePosition(),
                                              button, false, mModifiers);

            /*if (dropWidget != nullptr && dropWidget->cursor() != mCursor) {
                mCursor = dropWidget->cursor();
                glfwSetCursor(mGLFWWindow, mCursors[(int) mCursor]);
            }*/

            if (action == SDL3::SDL_EVENT_MOUSE_BUTTON_DOWN && button == SDL_BUTTON_LEFT)
            {
                mDragWidget = findWidget(mMousePos);
                if (mDragWidget == this)
                    mDragWidget = nullptr;
                mDragActive = mDragWidget != nullptr;
                if (!mDragActive)
                    updateFocus(nullptr);
            }
            else
            {
                mDragActive = false;
                mDragWidget = nullptr;
            }

            return mouseButtonEvent(mMousePos, button, action == SDL3::SDL_EVENT_MOUSE_BUTTON_DOWN,
                                    mModifiers);
        }
        catch (const std::exception& e)
        {
            std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
            abort();
        }
    }

    bool Screen::keyCallbackEvent(int key, int scancode, int action, int mods)
    {
        mLastInteraction = SDL3::SDL_GetTicks();
        try
        {
            return keyboardEvent(key, scancode, action, mods);
        }
        catch (const std::exception& e)
        {
            std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
            abort();
        }
    }

    bool Screen::charCallbackEvent(unsigned int codepoint)
    {
        mLastInteraction = SDL3::SDL_GetTicks();
        try
        {
            return keyboardCharacterEvent(codepoint);
        }
        catch (const std::exception& e)
        {
            std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
            abort();
        }
    }

    bool Screen::dropCallbackEvent(int count, const char** filenames)
    {
        std::vector<std::string> arg(count);
        for (int i = 0; i < count; ++i)
            arg[i] = filenames[i];
        return dropEvent(arg);
    }

    bool Screen::scrollCallbackEvent(double x, double y)
    {
        mLastInteraction = SDL3::SDL_GetTicks();
        try
        {
            if (mFocusPath.size() > 1)
            {
                const Window* window = dynamic_cast<Window*>(mFocusPath[mFocusPath.size() - 2]);
                if (window && window->modal())
                {
                    if (!window->contains(mMousePos))
                        return false;
                }
            }
            return scrollEvent(mMousePos, Vector2f((float)x, (float)y));
        }
        catch (const std::exception& e)
        {
            std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
            abort();
        }
    }

    bool Screen::resizeCallbackEvent(int, int)
    {
        Vector2i fbSize, size;
        // glfwGetFramebufferSize(mGLFWWindow, &fbSize[0], &fbSize[1]);
        SDL_GetWindowSize(m_sdl_window, &size[0], &size[1]);

        if (mFBSize == Vector2i(0, 0) || size == Vector2i(0, 0))
            return false;

        mFBSize = fbSize;
        mSize = size;
        mLastInteraction = SDL3::SDL_GetTicks();

        try
        {
            return resizeEvent(mSize);
        }
        catch (const std::exception& e)
        {
            std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
            abort();
        }
    }

    void Screen::updateFocus(Widget* widget)
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
            moveWindowToFront((Window*)window);
    }

    void Screen::disposeWindow(Window* window)
    {
        if (std::find(mFocusPath.begin(), mFocusPath.end(), window) != mFocusPath.end())
            mFocusPath.clear();
        if (mDragWidget == window)
            mDragWidget = nullptr;
        removeChild(window);
    }

    void Screen::centerWindow(Window* window)
    {
        if (window->size() == Vector2i{ 0, 0 })
        {
            window->setSize(window->preferredSize(m_sdl_renderer));
            window->performLayout(m_sdl_renderer);
        }
        window->setPosition((mSize - window->size()) / 2);
    }

    void Screen::moveWindowToFront(Window* window)
    {
        mChildren.erase(std::remove(mChildren.begin(), mChildren.end(), window), mChildren.end());
        mChildren.push_back(window);
        /* Brute force topological sort (no problem for a few windows..) */
        bool changed = false;
        do
        {
            size_t baseIndex = 0;
            for (size_t index = 0; index < mChildren.size(); ++index)
                if (mChildren[index] == window)
                    baseIndex = index;
            changed = false;
            for (size_t index = 0; index < mChildren.size(); ++index)
            {
                Popup* pw = dynamic_cast<Popup*>(mChildren[index]);
                if (pw && pw->parentWindow() == window && index < baseIndex)
                {
                    moveWindowToFront(pw);
                    changed = true;
                    break;
                }
            }
        }
        while (changed);
    }

    void Screen::performLayout(SDL3::SDL_Renderer* ctx)
    {
        Widget::performLayout(ctx);
    }

    void Screen::performLayout()
    {
        Widget::performLayout(m_sdl_renderer);
    }

}
