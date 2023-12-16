#include <array>
#include <cmath>
#include <numeric>

#include "gui/entypo.hpp"
#include "gui/tabheader.hpp"
#include "gui/theme.hpp"
#include "sdl/defs.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL.h>
SDL_C_LIB_END

#pragma warning(disable : 4838)

namespace rl::gui {
    TabHeader::TabButton::TabButton(TabHeader& header, const std::string& label)
        : m_tab_header(&header)
        , m_label(label)
    {
        m_label_texture.dirty = true;
    }

    Vector2i TabHeader::TabButton::preferred_size(SDL3::SDL_Renderer* ctx) const
    {
        // No need to call nvg font related functions since this is done by the tab header
        // implementation
        int w, h;
        auto theme = const_cast<TabButton*>(this)->m_tab_header->theme();
        theme->get_utf8_bounds("sans", m_tab_header->font_size(), m_label.c_str(), &w, &h);

        int buttonWidth = w + 2 * m_tab_header->theme()->m_tab_button_horizontal_padding;
        int buttonHeight = h + 2 * m_tab_header->theme()->m_tab_button_vertical_padding;
        return Vector2i(buttonWidth, buttonHeight);
    }

    void TabHeader::TabButton::calculateVisibleString(SDL3::SDL_Renderer* renderer)
    {
        // The size must have been set in by the enclosing tab header.
        std::string displayedText = m_tab_header->theme()->break_text(
            renderer, m_label.c_str(), "sans", m_tab_header->font_size(), m_size.x - 10);

        m_visible_text.first = m_label.c_str();

        // Check to see if the text need to be truncated.
        if (displayedText.size() != m_label.size())
        {
            int truncatedWidth = m_tab_header->theme()->get_text_width(
                "sans", m_tab_header->font_size(), displayedText.c_str());
            int dotsWidth = m_tab_header->theme()->get_text_width("sans", m_tab_header->font_size(),
                                                                  dots);

            // Remember the truncated width to know where to display the dots.
            m_visible_width = truncatedWidth;
            m_visible_text.last = m_label.c_str() + displayedText.size();
        }
        else
        {
            m_visible_text.last = nullptr;
            m_visible_width = 0;
        }
    }

    void TabHeader::TabButton::drawAtPosition(SDL3::SDL_Renderer* renderer,
                                              const Vector2i& position, bool active)
    {
        int xPos = position.x;
        int yPos = position.y;
        int width = m_size.x;
        int height = m_size.y;
        auto theme = m_tab_header->theme();

        int lx = m_tab_header->get_absolute_left();
        int ly = m_tab_header->get_absolute_top();

        // nvgSave(ctx);
        // nvgIntersectScissor(ctx, xPos, yPos, width+1, height);
        if (!active)
        {
            // Background gradients
            Color gradTop = theme->m_button_gradient_top_pushed;
            Color gradBot = theme->m_button_gradient_bot_pushed;

            // Draw the background.
            // nvgBeginPath(ctx);
            SDL3::SDL_FRect trect{
                static_cast<float>(lx + xPos + 1),
                static_cast<float>(ly + yPos + 1),
                static_cast<float>(width - 1),
                static_cast<float>(height - 1),
            };
            SDL3::SDL_Color b = gradTop.sdl_color();
            SDL3::SDL_Color bt = gradBot.sdl_color();

            SDL3::SDL_SetRenderDrawColor(renderer, b.r, b.g, b.b, b.a);
            SDL3::SDL_RenderFillRect(renderer, &trect);
        }

        if (active)
        {
            SDL3::SDL_Color bl = theme->m_border_light.sdl_color();
            SDL3::SDL_FRect blr{
                static_cast<float>(lx + xPos + 1),
                static_cast<float>(ly + yPos + 2),
                static_cast<float>(width),
                static_cast<float>(height),
            };

            SDL3::SDL_SetRenderDrawColor(renderer, bl.r, bl.g, bl.b, bl.a);
            SDL3::SDL_RenderLine(renderer, blr.x, blr.y, blr.x, blr.y + blr.h);
            SDL3::SDL_RenderLine(renderer, blr.x, blr.y, blr.x + blr.w, blr.y);
            SDL3::SDL_RenderLine(renderer, blr.x + blr.w, blr.y, blr.x + blr.w, blr.y + blr.h);

            SDL3::SDL_Color bd = theme->m_border_dark.sdl_color();
            SDL3::SDL_FRect bdr{
                static_cast<float>(lx + xPos + 1),
                static_cast<float>(ly + yPos + 1),
                static_cast<float>(width),
                static_cast<float>(height),
            };

            SDL3::SDL_SetRenderDrawColor(renderer, bd.r, bd.g, bd.b, bd.a);
            SDL3::SDL_RenderLine(renderer, bdr.x, bdr.y, bdr.x, bdr.y + bdr.h);
            SDL3::SDL_RenderLine(renderer, bdr.x, bdr.y, bdr.x + bdr.w, bdr.y);
            SDL3::SDL_RenderLine(renderer, bdr.x + bdr.w, bdr.y, bdr.x + bdr.w, bdr.y + bdr.h);
        }
        else
        {
            SDL3::SDL_Color bd = theme->m_border_dark.sdl_color();
            SDL3::SDL_FRect bdr{
                static_cast<float>(lx + xPos + 1),
                static_cast<float>(ly + yPos + 2),
                static_cast<float>(width),
                static_cast<float>(height - 1),
            };

            SDL3::SDL_SetRenderDrawColor(renderer, bd.r, bd.g, bd.b, bd.a);
            SDL3::SDL_RenderLine(renderer, bdr.x, bdr.y, bdr.x, bdr.y + bdr.h);
            SDL3::SDL_RenderLine(renderer, bdr.x, bdr.y, bdr.x + bdr.w, bdr.y);
            SDL3::SDL_RenderLine(renderer, bdr.x + bdr.w, bdr.y, bdr.x + bdr.w, bdr.y + bdr.h);
        }

        // Draw the text with some padding
        if (m_label_texture.dirty)
        {
            std::string lb(m_visible_text.first,
                           m_visible_text.last ? m_visible_text.last - m_visible_text.first : 0xff);

            if (m_visible_text.last != nullptr)
                lb += dots;
            m_tab_header->theme()->get_texture_and_rect_utf8(
                renderer, m_label_texture, 0, 0, lb.c_str(), "sans", m_tab_header->font_size(),
                m_tab_header->theme()->m_text_color);
        }

        if (m_label_texture.tex)
        {
            int textX = m_tab_header->get_absolute_left() + xPos +
                        m_tab_header->theme()->m_tab_button_horizontal_padding;
            int textY = m_tab_header->get_absolute_top() + yPos +
                        m_tab_header->theme()->m_tab_button_vertical_padding + (active ? 1 : -2);

            SDL_RenderCopy(renderer, m_label_texture, Vector2i(textX, textY));
        }
    }

    void TabHeader::TabButton::drawActiveBorderAt(
        SDL3::SDL_Renderer* renderer, const Vector2i& position, float offset, const Color& color)
    {
        int xPos = position.x;
        int yPos = position.y;
        int width = m_size.x;
        int height = m_size.y;

        SDL3::SDL_Color c = color.sdl_color();
        SDL3::SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
        SDL3::SDL_RenderLine(renderer, xPos + offset, yPos + height + offset, xPos + offset,
                             yPos + offset);
        SDL3::SDL_RenderLine(renderer, xPos + offset, yPos + offset, xPos + width - offset,
                             yPos + offset);
        SDL3::SDL_RenderLine(renderer, xPos + width - offset, yPos + offset, xPos + width - offset,
                             yPos + height + offset);
    }

    void TabHeader::TabButton::drawInactiveBorderAt(
        SDL3::SDL_Renderer* renderer, const Vector2i& position, float offset, const Color& color)
    {
        int xPos = position.x;
        int yPos = position.y;
        int width = m_size.x;
        int height = m_size.y;

        SDL3::SDL_Color c = color.sdl_color();
        SDL3::SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
        SDL3::SDL_FRect r{ std::round(xPos + offset), std::round(yPos + offset),
                           std::round(width - offset), std::round(height - offset) };
        SDL3::SDL_RenderRect(renderer, &r);
    }

    TabHeader::TabHeader(Widget* parent, const std::string& font)
        : Widget(parent)
        , m_font(font)
    {
    }

    void TabHeader::set_active_tab(size_t tab_index)
    {
        runtime_assert(tab_index < this->tab_count(), "tab button index out of bounds");
        m_active_tab_idx = tab_index;
        if (m_active_header_changed_callback)
            m_active_header_changed_callback(tab_index);
    }

    size_t TabHeader::active_tab() const
    {
        return m_active_tab_idx;
    }

    bool TabHeader::isTabVisible(size_t index) const
    {
        return index >= m_visible_start && index < m_visible_end;
    }

    void TabHeader::add_tab(const std::string& label)
    {
        this->add_tab(this->tab_count(), label);
    }

    void TabHeader::add_tab(size_t index, const std::string& label)
    {
        runtime_assert(index <= tab_count(), "tab button index out of bounds");
        mTabButtons.insert(std::next(mTabButtons.begin(), index), TabButton(*this, label));
        set_active_tab(index);
    }

    int TabHeader::remove_tab(const std::string& label)
    {
        auto element = std::find_if(mTabButtons.begin(), mTabButtons.end(),
                                    [&](const TabButton& tb) {
                                        return label == tb.label();
                                    });
        int index = std::distance(mTabButtons.begin(), element);
        if (element == mTabButtons.end())
            return -1;
        mTabButtons.erase(element);
        if (index == m_active_tab_idx && index != 0)
            set_active_tab(index - 1);
        return index;
    }

    void TabHeader::remove_tab(size_t index)
    {
        runtime_assert(index < this->tab_count(), "tab button index out of bounds");
        mTabButtons.erase(std::next(mTabButtons.begin(), index));
        if (index == m_active_tab_idx && index != 0)
            set_active_tab(index - 1);
    }

    const std::string& TabHeader::tab_label_at(size_t index) const
    {
        runtime_assert(index < this->tab_count(), "tab button index out of bounds");
        return mTabButtons[index].label();
    }

    int TabHeader::tab_index(const std::string& label)
    {
        auto it = std::find_if(mTabButtons.begin(), mTabButtons.end(), [&](const TabButton& tb) {
            return label == tb.label();
        });
        if (it == mTabButtons.end())
            return -1;
        return static_cast<int>(it - mTabButtons.begin());
    }

    void TabHeader::ensure_tab_visible(size_t index)
    {
        auto visibleArea{ this->visible_button_area() };
        auto visibleWidth{ visibleArea.second.x - visibleArea.first.x };
        int allowedVisibleWidth{ m_size.x - 2 * theme()->m_tab_control_width };

        runtime_assert(allowedVisibleWidth >= visibleWidth, "allowable visible width exceeded");
        runtime_assert(index >= 0 && index < mTabButtons.size(), "tab button index out of bounds");

        auto first{ this->visibleBegin() };
        auto last{ this->visibleEnd() };
        auto goal{ this->tabIterator(index) };

        // Reach the goal tab with the visible range.
        if (goal < first)
        {
            do
            {
                --first;
                visibleWidth += first->size().x;
            }
            while (goal < first);

            while (allowedVisibleWidth < visibleWidth)
            {
                --last;
                visibleWidth -= last->size().x;
            }
        }
        else if (goal >= last)
        {
            do
            {
                visibleWidth += last->size().x;
                ++last;
            }
            while (goal >= last);

            while (allowedVisibleWidth < visibleWidth)
            {
                visibleWidth -= first->size().x;
                ++first;
            }
        }

        // Check if it is possible to expand the visible range on either side.
        while (first != mTabButtons.begin() &&
               std::next(first, -1)->size().x < allowedVisibleWidth - visibleWidth)
        {
            --first;
            visibleWidth += first->size().x;
        }

        while (last != mTabButtons.end() && last->size().x < allowedVisibleWidth - visibleWidth)
        {
            visibleWidth += last->size().x;
            ++last;
        }

        m_visible_start = (int)std::distance(mTabButtons.begin(), first);
        m_visible_end = (int)std::distance(mTabButtons.begin(), last);
    }

    std::pair<Vector2i, Vector2i> TabHeader::visible_button_area() const
    {
        if (m_visible_start == m_visible_end)
            return { { 0, 0 }, { 0, 0 } };
        auto topLeft = m_pos + Vector2i(theme()->m_tab_control_width, 0);
        auto width = std::accumulate(visibleBegin(), visibleEnd(), theme()->m_tab_control_width,
                                     [](int acc, const TabButton& tb) {
                                         return acc + tb.size().x;
                                     });
        auto bottomRight = m_pos + Vector2i{ width, m_size.y };
        return { topLeft, bottomRight };
    }

    std::pair<Vector2i, Vector2i> TabHeader::active_button_area() const
    {
        if (m_visible_start == m_visible_end || m_active_tab_idx < m_visible_start ||
            m_active_tab_idx >= m_visible_end)
        {
            return {
                { 0, 0 },
                { 0, 0 },
            };
        }

        auto width{
            std::accumulate(visibleBegin(), activeIterator(), theme()->m_tab_control_width,
                            [](int acc, const TabButton& tb) {
                                return acc + tb.size().x;
                            }),
        };
        auto topLeft = m_pos + Vector2i{ width, 0 };
        auto bottomRight = m_pos + Vector2i{ width + activeIterator()->size().x, m_size.y };
        return { topLeft, bottomRight };
    }

    void TabHeader::perform_layout(SDL3::SDL_Renderer* ctx)
    {
        Widget::perform_layout(ctx);

        Vector2i currentPosition(0, 0);
        // Place the tab buttons relative to the beginning of the tab header.
        for (auto& tab : mTabButtons)
        {
            auto tabPreferred = tab.preferred_size(ctx);
            if (tabPreferred.x < theme()->m_tab_min_button_width)
                tabPreferred.x = theme()->m_tab_min_button_width;
            else if (tabPreferred.x > theme()->m_tab_max_button_width)
                tabPreferred.x = theme()->m_tab_max_button_width;
            tab.set_size(tabPreferred);
            tab.calculateVisibleString(nullptr);
            currentPosition.x += tabPreferred.x;
        }
        calculateVisibleEnd();
        if (m_visible_start != 0 || m_visible_end != tab_count())
            mOverflowing = true;
    }

    Vector2i TabHeader::preferred_size(SDL3::SDL_Renderer* ctx) const
    {
        // Set up the nvg context for measuring the text inside the tab buttons.
        // nvgFontFace(ctx, m_font.c_str());
        // nvgFontSize(ctx, font_size());
        // nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
        Vector2i size = Vector2i(2 * theme()->m_tab_control_width, 0);
        for (auto& tab : mTabButtons)
        {
            auto tabPreferred = tab.preferred_size(ctx);
            if (tabPreferred.x < theme()->m_tab_min_button_width)
                tabPreferred.x = theme()->m_tab_min_button_width;
            else if (tabPreferred.x > theme()->m_tab_max_button_width)
                tabPreferred.x = theme()->m_tab_max_button_width;
            size.x += tabPreferred.x;
            size.y = std::max(size.y, tabPreferred.y);
        }
        return size;
    }

    bool TabHeader::mouse_button_event(const Vector2i& p, int button, bool down, int modifiers)
    {
        Widget::mouse_button_event(p, button, down, modifiers);
        if (button == SDL_BUTTON_LEFT && down)
        {
            switch (locateClick(p))
            {
                case ClickLocation::LeftControls:
                    onArrowLeft();
                    return true;
                case ClickLocation::RightControls:
                    onArrowRight();
                    return true;
                case ClickLocation::TabButtons:
                    auto first = visibleBegin();
                    auto last = visibleEnd();
                    int currentPosition = theme()->m_tab_control_width;
                    int endPosition = p.x;
                    auto firstInvisible = std::find_if(
                        first, last, [&currentPosition, endPosition](const TabButton& tb) {
                            currentPosition += tb.size().x;
                            return currentPosition > endPosition;
                        });

                    // Did not click on any of the tab buttons
                    if (firstInvisible == last)
                        return true;

                    // Update the active tab and invoke the callback.
                    set_active_tab((int)std::distance(mTabButtons.begin(), firstInvisible));
                    return true;
            }
        }
        return false;
    }

    void TabHeader::draw(const std::unique_ptr<rl::Renderer>& renderer)
    {
    }

    void TabHeader::draw(SDL3::SDL_Renderer* renderer)
    {
        // Draw controls.
        Widget::draw(renderer);
        if (mOverflowing)
            drawControls(renderer);

        auto current = visibleBegin();
        auto last = visibleEnd();
        auto active = std::next(mTabButtons.begin(), m_active_tab_idx);
        Vector2i currentPosition = m_pos + Vector2i(theme()->m_tab_control_width, 0);

        // Flag to draw the active tab last. Looks a little bit better.
        bool drawActive = false;
        Vector2i activePosition{ 0, 0 };

        // Draw inactive visible buttons.
        for (; current != last; ++current)
        {
            if (current == active)
            {
                drawActive = true;
                activePosition = currentPosition;
            }
            else
            {
                current->drawAtPosition(renderer, currentPosition, false);
            }
            currentPosition.x += current->size().x;
        }

        // Draw active visible button.
        if (drawActive)
            active->drawAtPosition(renderer, activePosition, true);
    }

    void TabHeader::calculateVisibleEnd()
    {
        auto first = visibleBegin();
        auto last = mTabButtons.end();
        int currentPosition = theme()->m_tab_control_width;
        int lastPosition = m_size.x - theme()->m_tab_control_width;
        auto firstInvisible = std::find_if(first, last,
                                           [&currentPosition, lastPosition](const TabButton& tb) {
                                               currentPosition += tb.size().x;
                                               return currentPosition > lastPosition;
                                           });
        m_visible_end = (int)std::distance(mTabButtons.begin(), firstInvisible);
    }

    void TabHeader::drawControls(SDL3::SDL_Renderer* renderer)
    {
        // Left button.
        int lactive = m_visible_start != 0 ? 1 : 0;
        int ractive = (m_visible_end != tab_count()) ? 1 : 0;

        // Draw the arrow.
        if (_lastLeftActive != lactive || _lastRightActive != ractive)
        {
            int fontSize = m_font_size == -1 ? m_theme->m_button_font_size : m_font_size;
            float ih = fontSize;
            ih *= 1.5f;
            if (_lastLeftActive != lactive)
            {
                auto iconLeft = utf8(ENTYPO_ICON_LEFT_BOLD);
                m_theme->get_texture_and_rect_utf8(
                    renderer, _leftIcon, 0, 0, iconLeft.data(), "icons", (size_t)ih,
                    lactive ? m_theme->m_text_color : m_theme->m_button_gradient_bot_pushed);
            }

            if (_lastRightActive != ractive)
            {
                auto iconRight = utf8(ENTYPO_ICON_RIGHT_BOLD);
                m_theme->get_texture_and_rect_utf8(
                    renderer, _rightIcon, 0, 0, iconRight.data(), "icons", (size_t)ih,
                    ractive ? m_theme->m_text_color : m_theme->m_button_gradient_bot_pushed);
            }

            _lastLeftActive = lactive;
            _lastRightActive = ractive;
        }

        float yScaleLeft = 0.5f;
        float xScaleLeft = 0.2f;
        if (_leftIcon.tex)
        {
            Vector2f leftIconPos = absolute_position().tofloat();
            leftIconPos += m_pos.tofloat() + Vector2f{ xScaleLeft * theme()->m_tab_control_width,
                                                       yScaleLeft * m_size.y };
            SDL_RenderCopy(
                renderer, _leftIcon,
                Vector2i(leftIconPos.x - _leftIcon.w() / 2, leftIconPos.y - _leftIcon.h() / 2));
        }

        // Draw the arrow.
        if (_rightIcon.tex)
        {
            float yScaleRight = 0.5f;
            float xScaleRight = 1.0f - xScaleLeft - _rightIcon.w() / theme()->m_tab_control_width;
            Vector2f leftControlsPos = absolute_position().tofloat();
            leftControlsPos += m_pos.tofloat() +
                               Vector2f(m_size.x - theme()->m_tab_control_width, 0);
            Vector2f rightIconPos = leftControlsPos +
                                    Vector2f(xScaleRight * theme()->m_tab_control_width,
                                             yScaleRight * m_size.tofloat().y);
            SDL_RenderCopy(renderer, _rightIcon,
                           Vector2i(rightIconPos.x - _rightIcon.w() / 2,
                                    rightIconPos.y - _rightIcon.h() / 2 + 1));
        }
    }

    TabHeader::ClickLocation TabHeader::locateClick(const Vector2i& p)
    {
        Vector2i leftDistance = p - m_pos;
        bool hitLeft = leftDistance.positive() &&
                       leftDistance.lessOrEq({ theme()->m_tab_control_width, m_size.y });
        if (hitLeft)
            return ClickLocation::LeftControls;
        auto rightDistance = p - (m_pos + Vector2i{ m_size.x - theme()->m_tab_control_width, 0 });
        bool hitRight = rightDistance.positive() &&
                        rightDistance.lessOrEq({ theme()->m_tab_control_width, m_size.y });
        if (hitRight)
            return ClickLocation::RightControls;
        return ClickLocation::TabButtons;
    }

    void TabHeader::onArrowLeft()
    {
        if (m_visible_start == 0)
            return;
        --m_visible_start;
        calculateVisibleEnd();
    }

    void TabHeader::onArrowRight()
    {
        if (m_visible_end == tab_count())
            return;
        ++m_visible_start;
        calculateVisibleEnd();
    }
}
