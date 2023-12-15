#include <array>

#include "gui/entypo.hpp"
#include "gui/popupbutton.hpp"
#include "gui/theme.hpp"

namespace rl::gui {
    PopupButton::PopupButton(Widget* parent, const std::string& caption, int buttonIcon,
                             int chevronIcon)
        : Button(parent, caption, buttonIcon)
        , mChevronIcon(chevronIcon)
    {
        setFlags(Flags::ToggleButton | Flags::PopupButton);

        Window* parentWindow = window();
        mPopup = new Popup(parentWindow->parent(), window());
        mPopup->set_size(Vector2i(320, 250));
        mPopup->set_visible(false);
    }

    Vector2i PopupButton::preferredSize(SDL3::SDL_Renderer* ctx) const
    {
        return Button::preferredSize(ctx) + Vector2i(15, 0);
    }

    void PopupButton::draw(SDL3::SDL_Renderer* renderer)
    {
        if (!m_enabled && mPushed)
            mPushed = false;

        mPopup->set_visible(mPushed);
        Button::draw(renderer);

        if (mChevronIcon)
        {
            if (_chevronTex.dirty)
            {
                auto icon = utf8(mChevronIcon);
                Color textColor = { (mTextColor.a() == 0.0f) ? m_theme->mTextColor : mTextColor };
                if (!m_enabled)
                    textColor = m_theme->mDisabledTextColor;
                int fntsize = (int)((m_font_size < 0 ? m_theme->mButtonFontSize : m_font_size) *
                                    1.5f);
                m_theme->getTexAndRectUtf8(renderer, _chevronTex, 0, 0, icon.data(), "icons",
                                           fntsize, textColor);
            }

            auto&& ap = absolute_position();
            auto&& vec = ap + Vector2i(m_size.x - _chevronTex.w() - 8, int(m_size.y * 0.5f - 1.0f));
            SDL3::SDL_FRect rect{
                static_cast<float>(vec.x),
                static_cast<float>(vec.y),
                0.0f,
                0.0f,
            };
            SDL3::SDL_RenderTexture(renderer, _chevronTex.tex, &rect, nullptr);
        }
    }

    void PopupButton::perform_layout(SDL3::SDL_Renderer* ctx)
    {
        Widget::perform_layout(ctx);

        const Window* parentWindow = window();

        mPopup->setAnchorPos(
            Vector2i(parentWindow->width() + 15,
                     absolute_position().y - parentWindow->relative_position().y + m_size.y / 2));
    }
}
