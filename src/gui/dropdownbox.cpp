#include <algorithm>
#include <array>
#include <cassert>
#include <string>

#include "gui/button.hpp"
#include "gui/common.hpp"
#include "gui/dropdownbox.hpp"
#include "gui/layout.hpp"
#include "gui/nanovg.h"
#include "sdl/defs.hpp"

#define NANOVG_RT_IMPLEMENTATION
#define NANORT_IMPLEMENTATION
#include "gui/nanovg_rt.h"

SDL_C_LIB_BEGIN
#include <SDL3/SDL.h>
#include <SDL3/SDL_render.h>
SDL_C_LIB_END

namespace rl::gui {
    class DropdownListItem : public Button
    {
    public:
        bool mInlist = true;

        DropdownListItem(Widget* parent, const std::string& str, bool inlist = true)
            : Button(parent, str)
            , mInlist(inlist)
        {
        }

        void renderBodyTexture(NVGcontext*& ctx, int& realw, int& realh) override
        {
            int ww = this->width();
            int hh = this->height();
            ctx = nvgCreateRT(NVG_DEBUG, ww + 2, hh + 2, 0);

            float pxRatio = 1.0f;
            realw = ww + 2;
            realh = hh + 2;
            nvgBeginFrame(ctx, realw, realh, pxRatio);

            if (!mInlist)
            {
                Color gradTop = m_theme->mButtonGradientTopPushed;
                Color gradBot = m_theme->mButtonGradientBotPushed;

                nvgBeginPath(ctx);

                nvgRoundedRect(ctx, 1.0f, 1.0f, ww - 2.0f, hh - 2.0f,
                               m_theme->mButtonCornerRadius - 1.0f);

                if (mBackgroundColor.a() != 0)
                {
                    Color rgb = mBackgroundColor.rgb();
                    rgb.setAlpha(1.f);
                    nvgFillColor(ctx, rgb.toNvgColor());
                    nvgFill(ctx);
                    gradTop.a() = gradBot.a() = 0.8f;
                }

                NVGpaint bg = nvgLinearGradient(ctx, 0, 0, 0, hh, gradTop.toNvgColor(),
                                                gradBot.toNvgColor());

                nvgFillPaint(ctx, bg);
                nvgFill(ctx);

                nvgBeginPath(ctx);
                nvgStrokeWidth(ctx, 1.0f);
                nvgRoundedRect(ctx, 0.5f, 0.5f, ww - 1, hh, m_theme->mButtonCornerRadius);
                nvgStrokeColor(ctx, m_theme->mBorderLight.toNvgColor());
                nvgStroke(ctx);

                nvgBeginPath(ctx);
                nvgRoundedRect(ctx, 0.5f, 0.5f, ww - 1, hh, m_theme->mButtonCornerRadius);
                nvgStrokeColor(ctx, m_theme->mBorderDark.toNvgColor());
                nvgStroke(ctx);
            }
            else if (m_mouse_focus && m_enabled)
            {
                Color gradTop = m_theme->mButtonGradientTopFocused;
                Color gradBot = m_theme->mButtonGradientBotFocused;

                nvgBeginPath(ctx);

                nvgRoundedRect(ctx, 1, 1, ww - 2, hh - 2, m_theme->mButtonCornerRadius - 1);

                if (mBackgroundColor.a() != 0)
                {
                    Color rgb = mBackgroundColor.rgb();
                    rgb.setAlpha(1.f);
                    nvgFillColor(ctx, rgb.toNvgColor());
                    nvgFill(ctx);
                    if (mPushed)
                        gradTop.a() = gradBot.a() = 0.8f;
                    else
                    {
                        double v = 1 - mBackgroundColor.a();
                        gradTop.a() = gradBot.a() = m_enabled ? v : v * .5f + .5f;
                    }
                }

                NVGpaint bg = nvgLinearGradient(ctx, 0, 0, 0, hh, gradTop.toNvgColor(),
                                                gradBot.toNvgColor());

                nvgFillPaint(ctx, bg);
                nvgFill(ctx);
            }

            if (mPushed && mInlist)
            {
                Color textColor = mTextColor.a() == 0 ? m_theme->mTextColor : mTextColor;
                Vector2f center = m_size.cast<float>() * 0.5f;

                nvgBeginPath(ctx);
                nvgCircle(ctx, width() * 0.05f, center.y, 2);
                nvgFillColor(ctx, textColor.toNvgColor());
                nvgFill(ctx);
            }

            nvgEndFrame(ctx);
        }

        Vector2i getTextOffset() const override
        {
            return Vector2i(0, 0);
        }
    };

    class DropdownPopup : public Popup
    {
    public:
        int preferredWidth = 0;

        DropdownPopup(Widget* parent, Window* parentWindow)
            : Popup(parent, parentWindow)
        {
            _anchorDx = 0;
        }

        float targetPath = 0;

        void hide()
        {
            targetPath = 0;
        }

        Vector2i preferredSize(SDL3::SDL_Renderer* ctx) const override
        {
            Vector2i result = Popup::preferredSize(ctx);
            result.x = preferredWidth;
            return result;
        }

        void refreshRelativePlacement() override
        {
            Popup::refreshRelativePlacement();
            m_visible &= mParentWindow->visible_recursive();

            Widget* widget = this;
            while (widget->parent() != nullptr)
                widget = widget->parent();
            Screen* screen = (Screen*)widget;
            Vector2i screenSize = screen->size();

            m_pos = mParentWindow->relative_position() + mAnchorPos;
            m_pos = Vector2i(m_pos.x, std::min(m_pos.y, screen->size().y - m_size.y));
        }

        void updateCaption(const std::string& caption)
        {
            if (m_children.size() > 0)
            {
                auto* btn = dynamic_cast<Button*>(m_children[0]);
                btn->setCaption(caption);
            }
        }

        void updateVisible(bool visible)
        {
            if (!visible)
            {
                if (path > 0)
                    path -= 0.15f;
                if (path <= 0)
                    path = 0.f;
            }
            else
            {
                if (path < 1.f)
                    path += 0.15f;
                if (path > 1.f)
                    path = 1.f;
            }

            m_visible = path > 0;
        }

        float path = 0.f;

        int clamp(int val, int min, int max)
        {
            return val < min ? min : (val > max ? max : val);
        }

        void rendereBodyTexture(NVGcontext*& ctx, int& realw, int& realh, int dx) override
        {
            int ds = 1, cr = m_theme->mWindowCornerRadius;
            int ww = m_fixed_size.x > 0 ? m_fixed_size.x : m_size.x;
            int hh = height();
            int dy = 0;
            int xadd = 1;

            int headerH = m_children[0]->height();
            int realH = clamp(m_size.y * path, headerH, m_size.y);

            Vector2i offset(dx + ds, dy + ds);

            realw = ww + 2 * ds + dx + xadd;  // with + 2*shadow + 2*boder + offset
            realh = hh + 2 * ds + dy + xadd;

            ctx = nvgCreateRT(NVG_DEBUG, realw, realh, 0);

            float pxRatio = 1.0f;
            nvgBeginFrame(ctx, realw, realh, pxRatio);

            // Draw a drop shadow
            NVGpaint shadowPaint = nvgBoxGradient(ctx, 0, 0, realw, realh, cr * 2, ds * 2,
                                                  m_theme->mDropShadow.toNvgColor(),
                                                  m_theme->mTransparent.toNvgColor());

            nvgBeginPath(ctx);
            nvgRect(ctx, 0, 0, ww + 2 * ds, hh + 2 * ds);
            // nvgRoundedRect(ctx, 0, 0, ww + 2 * ds, hh + 2 * ds, cr);
            // nvgPathWinding(ctx, NVG_HOLE);
            nvgFillPaint(ctx, shadowPaint);
            nvgFill(ctx);

            // Draw window
            nvgBeginPath(ctx);
            nvgRect(ctx, offset.x, offset.y, ww, hh);

            nvgFillColor(ctx, m_theme->mWindowPopup.toNvgColor());
            nvgFill(ctx);

            nvgEndFrame(ctx);
        }

        Vector2i getOverrideBodyPos() override
        {
            Vector2i ap = absolute_position();
            int ds = 2;  // m_theme->mWindowDropShadowSize;
            return ap - Vector2i(ds, ds);
        }

        void draw(SDL3::SDL_Renderer* renderer) override
        {
            refreshRelativePlacement();

            if (!m_visible || m_children.empty())
                return;

            drawBody(renderer);

            int ds = 1, cr = m_theme->mWindowCornerRadius;
            int ww = m_fixed_size.x > 0 ? m_fixed_size.x : m_size.x;

            int headerH = m_children[0]->height();
            int realH = clamp(m_size.y * path, headerH, m_size.y);

            // if (mChildren.size() > 1)
            // {
            //   nvgBeginPath(ctx);
            //
            //   Vector2i fp = mPos + mChildren[1]->relative_position();
            //   NVGpaint bg = nvgLinearGradient(ctx, fp.x(), fp.y(), fp.x(), fp.y() + 12 ,
            //                                   m_theme->mBorderMedium, m_theme->mTransparent);
            //   nvgRect(ctx, fp.x(), fp.y(), ww, 12);
            //   nvgFillPaint(ctx, bg);
            //   nvgFill(ctx);
            // }

            Widget::draw(renderer);
        }
    };

    DropdownBox::DropdownBox(Widget* parent)
        : PopupButton(parent)
    {
        mSelectedIndex = 0;
        Window* parentWindow = window();
        parentWindow->parent()->remove_child(mPopup);

        mPopup = new DropdownPopup(parentWindow->parent(), window());
        mPopup->set_size(Vector2i(320, 250));
        mPopup->set_visible(false);
        mPopup->setAnchorPos(Vector2i(0, 0));
    }

    DropdownBox::DropdownBox(Widget* parent, const std::vector<std::string>& items)
        : DropdownBox(parent)
    {
        setItems(items);
    }

    DropdownBox::DropdownBox(Widget* parent, const std::vector<std::string>& items,
                             const std::vector<std::string>& itemsShort)
        : DropdownBox(parent)
    {
        setItems(items, itemsShort);
    }

    void DropdownBox::perform_layout(SDL3::SDL_Renderer* renderer)
    {
        PopupButton::perform_layout(renderer);

        auto* dpopup = dynamic_cast<DropdownPopup*>(mPopup);
        if (dpopup)
        {
            dpopup->setAnchorPos(relative_position());
            dpopup->preferredWidth = width();
        }
    }

    void DropdownBox::setSelectedIndex(int idx)
    {
        if (mItemsShort.empty())
            return;

        const std::vector<Widget*>& children = popup().children();
        ((Button*)children[mSelectedIndex + 1])->setPushed(false);
        ((Button*)children[idx + 1])->setPushed(true);
        mSelectedIndex = idx;
        setCaption(mItemsShort[idx]);
        ((DropdownPopup*)mPopup)->updateCaption(mItemsShort[idx]);
    }

    void DropdownBox::setItems(const std::vector<std::string>& items,
                               const std::vector<std::string>& itemsShort)
    {
        assert(items.size() == itemsShort.size());
        mItems = items;
        mItemsShort = itemsShort;
        if (mSelectedIndex < 0 || mSelectedIndex >= (int)items.size())
            mSelectedIndex = 0;

        while (mPopup->child_count() != 0)
            mPopup->remove_child(mPopup->child_count() - 1);

        mPopup->set_layout(new GroupLayout(0, 0, 0, 0));
        if (!items.empty())
        {
            DropdownListItem* button = new DropdownListItem(mPopup, items[mSelectedIndex], false);
            button->setPushed(false);
            button->setCallback([&] {
                setPushed(false);
                popup().set_visible(false);
            });
        }

        int index = 0;
        for (const auto& str : items)
        {
            DropdownListItem* button = new DropdownListItem(mPopup, str);
            button->setFlags(Button::RadioButton);
            button->setCallback([&, index] {
                setSelectedIndex(index);
                setPushed(false);
                if (m_popup_callback)
                    m_popup_callback(index);
            });
            index++;
        }
        setSelectedIndex(mSelectedIndex);
    }

    bool DropdownBox::mouseButtonEvent(const Vector2i& p, int button, bool down, int modifiers)
    {
        if (button == SDL_BUTTON_LEFT && m_enabled)
        {
            if (!mItems.empty())
            {
                auto* item = dynamic_cast<DropdownListItem*>(mPopup->get_child(0));
                if (item)
                    item->setCaption(mItems[mSelectedIndex]);
            }
        }

        return PopupButton::mouseButtonEvent(p, button, down, modifiers);
    }

    bool DropdownBox::scrollEvent(const Vector2i& p, const Vector2f& rel)
    {
        if (rel.y < 0)
        {
            setSelectedIndex(std::min(mSelectedIndex + 1, (int)(items().size() - 1)));
            if (m_popup_callback)
                m_popup_callback(mSelectedIndex);
            return true;
        }
        else if (rel.y > 0)
        {
            setSelectedIndex(std::max(mSelectedIndex - 1, 0));
            if (m_popup_callback)
                m_popup_callback(mSelectedIndex);
            return true;
        }
        return PopupButton::scrollEvent(p, rel);
    }

    void DropdownBox::draw(SDL3::SDL_Renderer* renderer)
    {
        if (!m_enabled && mPushed)
            mPushed = false;

        if (auto pp = dynamic_cast<DropdownPopup*>(mPopup))
            pp->updateVisible(mPushed);

        Button::draw(renderer);

        if (mChevronIcon)
        {
            auto icon = utf8(mChevronIcon);
            Color textColor = mTextColor.a() == 0 ? m_theme->mTextColor : mTextColor;

            /*  nvgFontSize(ctx, (mFontSize < 0 ? m_theme->mButtonFontSize : mFontSize) *
              icon_scale()); nvgFontFace(ctx, "icons"); nvgFillColor(ctx, mEnabled ? textColor :
              m_theme->mDisabledTextColor); nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);

              float iw = nvgTextBounds(ctx, 0, 0, icon.data(), nullptr, nullptr);
              Vector2f iconPos(0, mPos.y() + mSize.y() * 0.5f - 1);

              if (mPopup->side() == Popup::Right)
                iconPos[0] = mPos.x() + mSize.x() - iw - 8;
              else
                iconPos[0] = mPos.x() + 8;

              nvgText(ctx, iconPos.x(), iconPos.y(), icon.data(), nullptr); */
        }
    }
}
