#include <algorithm>
#include <array>
#include <cassert>
#include <string>

#include "core/assert.hpp"
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

        void render_body_texture(NVGcontext*& ctx, int& realw, int& realh) override
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
                Color gradTop = m_theme->m_button_gradient_top_pushed;
                Color gradBot = m_theme->m_button_gradient_bot_pushed;

                nvgBeginPath(ctx);

                nvgRoundedRect(ctx, 1.0f, 1.0f, ww - 2.0f, hh - 2.0f,
                               m_theme->m_button_corner_radius - 1.0f);

                if (m_background_color.a() != 0)
                {
                    Color rgb = m_background_color.rgb();
                    rgb.set_alpha(1.f);
                    nvgFillColor(ctx, rgb.to_nvg_color());
                    nvgFill(ctx);
                    gradTop.a() = gradBot.a() = 0.8f;
                }

                NVGpaint bg = nvgLinearGradient(ctx, 0, 0, 0, hh, gradTop.to_nvg_color(),
                                                gradBot.to_nvg_color());

                nvgFillPaint(ctx, bg);
                nvgFill(ctx);

                nvgBeginPath(ctx);
                nvgStrokeWidth(ctx, 1.0f);
                nvgRoundedRect(ctx, 0.5f, 0.5f, ww - 1, hh, m_theme->m_button_corner_radius);
                nvgStrokeColor(ctx, m_theme->m_border_light.to_nvg_color());
                nvgStroke(ctx);

                nvgBeginPath(ctx);
                nvgRoundedRect(ctx, 0.5f, 0.5f, ww - 1, hh, m_theme->m_button_corner_radius);
                nvgStrokeColor(ctx, m_theme->m_border_dark.to_nvg_color());
                nvgStroke(ctx);
            }
            else if (m_mouse_focus && m_enabled)
            {
                Color gradTop = m_theme->m_button_gradient_top_focused;
                Color gradBot = m_theme->m_button_gradient_bot_focused;

                nvgBeginPath(ctx);

                nvgRoundedRect(ctx, 1, 1, ww - 2, hh - 2, m_theme->m_button_corner_radius - 1);

                if (m_background_color.a() != 0)
                {
                    Color rgb = m_background_color.rgb();
                    rgb.set_alpha(1.f);
                    nvgFillColor(ctx, rgb.to_nvg_color());
                    nvgFill(ctx);
                    if (m_pushed)
                        gradTop.a() = gradBot.a() = 0.8f;
                    else
                    {
                        double v = 1 - m_background_color.a();
                        gradTop.a() = gradBot.a() = m_enabled ? v : v * .5f + .5f;
                    }
                }

                NVGpaint bg = nvgLinearGradient(ctx, 0, 0, 0, hh, gradTop.to_nvg_color(),
                                                gradBot.to_nvg_color());

                nvgFillPaint(ctx, bg);
                nvgFill(ctx);
            }

            if (m_pushed && mInlist)
            {
                Color text_color = m_text_color.a() == 0 ? m_theme->m_text_color : m_text_color;
                Vector2f center = m_size.cast<float>() * 0.5f;

                nvgBeginPath(ctx);
                nvgCircle(ctx, width() * 0.05f, center.y, 2);
                nvgFillColor(ctx, text_color.to_nvg_color());
                nvgFill(ctx);
            }

            nvgEndFrame(ctx);
        }

        Vector2i get_text_offset() const override
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

        Vector2i preferred_size(SDL3::SDL_Renderer* ctx) const override
        {
            Vector2i result = Popup::preferred_size(ctx);
            result.x = preferredWidth;
            return result;
        }

        void refresh_relative_placement() override
        {
            Popup::refresh_relative_placement();
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
                btn->set_caption(caption);
            }
        }

        void updateVisible(bool visible)
        {
            if (!visible)
            {
                if (m_path > 0)
                    m_path -= 0.15f;
                if (m_path <= 0)
                    m_path = 0.f;
            }
            else
            {
                if (m_path < 1.f)
                    m_path += 0.15f;
                if (m_path > 1.f)
                    m_path = 1.f;
            }

            m_visible = m_path > 0;
        }

        float m_path = 0.f;

        int clamp(int val, int min, int max)
        {
            return val < min ? min : (val > max ? max : val);
        }

        void rendereBodyTexture(NVGcontext*& ctx, int& realw, int& realh, int dx) override
        {
            int ds = 1, cr = m_theme->m_window_corner_radius;
            int ww = m_fixed_size.x > 0 ? m_fixed_size.x : m_size.x;
            int hh = height();
            int dy = 0;
            int xadd = 1;

            int headerH = m_children[0]->height();
            int realH = clamp(m_size.y * m_path, headerH, m_size.y);

            Vector2i offset(dx + ds, dy + ds);

            realw = ww + 2 * ds + dx + xadd;  // with + 2*shadow + 2*boder + offset
            realh = hh + 2 * ds + dy + xadd;

            ctx = nvgCreateRT(NVG_DEBUG, realw, realh, 0);

            float pxRatio = 1.0f;
            nvgBeginFrame(ctx, realw, realh, pxRatio);

            // Draw a drop shadow
            NVGpaint shadowPaint = nvgBoxGradient(ctx, 0, 0, realw, realh, cr * 2, ds * 2,
                                                  m_theme->m_drop_shadow.to_nvg_color(),
                                                  m_theme->m_transparent.to_nvg_color());

            nvgBeginPath(ctx);
            nvgRect(ctx, 0, 0, ww + 2 * ds, hh + 2 * ds);
            // nvgRoundedRect(ctx, 0, 0, ww + 2 * ds, hh + 2 * ds, cr);
            // nvgPathWinding(ctx, NVG_HOLE);
            nvgFillPaint(ctx, shadowPaint);
            nvgFill(ctx);

            // Draw window
            nvgBeginPath(ctx);
            nvgRect(ctx, offset.x, offset.y, ww, hh);

            nvgFillColor(ctx, m_theme->m_window_popup.to_nvg_color());
            nvgFill(ctx);

            nvgEndFrame(ctx);
        }

        Vector2i getOverrideBodyPos() override
        {
            Vector2i ap = absolute_position();
            int ds = 2;  // m_theme->m_window_drop_shadow_size;
            return ap - Vector2i(ds, ds);
        }

        void draw(SDL3::SDL_Renderer* renderer) override
        {
            refresh_relative_placement();

            if (!m_visible || m_children.empty())
                return;

            draw_body(renderer);

            int ds = 1, cr = m_theme->m_window_corner_radius;
            int ww = m_fixed_size.x > 0 ? m_fixed_size.x : m_size.x;

            int headerH = m_children[0]->height();
            int realH = clamp(m_size.y * m_path, headerH, m_size.y);

            // if (mChildren.size() > 1)
            // {
            //   nvgBeginPath(ctx);
            //
            //   Vector2i fp = mPos + mChildren[1]->relative_position();
            //   NVGpaint bg = nvgLinearGradient(ctx, fp.x(), fp.y(), fp.x(), fp.y() + 12 ,
            //                                   m_theme->m_border_medium, m_theme->m_transparent);
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

    void DropdownBox::set_selected_index(int idx)
    {
        if (mItemsShort.empty())
            return;

        const std::vector<Widget*>& children = popup().children();
        ((Button*)children[mSelectedIndex + 1])->set_pushed(false);
        ((Button*)children[idx + 1])->set_pushed(true);
        mSelectedIndex = idx;
        set_caption(mItemsShort[idx]);
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
            button->set_pushed(false);
            button->set_callback([&] {
                set_pushed(false);
                popup().set_visible(false);
            });
        }

        int index = 0;
        for (const auto& str : items)
        {
            DropdownListItem* button = new DropdownListItem(mPopup, str);
            button->set_flags(Button::RadioButton);
            button->set_callback([&, index] {
                set_selected_index(index);
                set_pushed(false);
                if (m_popup_callback)
                    m_popup_callback(index);
            });
            index++;
        }
        set_selected_index(mSelectedIndex);
    }

    bool DropdownBox::mouse_button_event(const Vector2i& p, int button, bool down, int modifiers)
    {
        if (button == SDL_BUTTON_LEFT && m_enabled)
        {
            if (!mItems.empty())
            {
                auto* item = dynamic_cast<DropdownListItem*>(mPopup->get_child(0));
                if (item)
                    item->set_caption(mItems[mSelectedIndex]);
            }
        }

        return PopupButton::mouse_button_event(p, button, down, modifiers);
    }

    bool DropdownBox::scroll_event(const Vector2i& p, const Vector2f& rel)
    {
        if (rel.y < 0)
        {
            set_selected_index(std::min(mSelectedIndex + 1, (int)(items().size() - 1)));
            if (m_popup_callback)
                m_popup_callback(mSelectedIndex);
            return true;
        }
        else if (rel.y > 0)
        {
            set_selected_index(std::max(mSelectedIndex - 1, 0));
            if (m_popup_callback)
                m_popup_callback(mSelectedIndex);
            return true;
        }
        return PopupButton::scroll_event(p, rel);
    }

    void DropdownBox::draw(SDL3::SDL_Renderer* renderer)
    {
        if (!m_enabled && m_pushed)
            m_pushed = false;

        if (auto pp = dynamic_cast<DropdownPopup*>(mPopup))
            pp->updateVisible(m_pushed);

        Button::draw(renderer);

        if (mChevronIcon)
        {
            auto icon = utf8(mChevronIcon);
            Color text_color = m_text_color.a() == 0 ? m_theme->m_text_color : m_text_color;

            /*  nvgFontSize(ctx, (mFontSize < 0 ? m_theme->m_button_font_size : mFontSize) *
              icon_scale()); nvgFontFace(ctx, "icons"); nvgFillColor(ctx, mEnabled ? textColor :
              m_theme->m_disabled_text_color); nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);

              float iw = nvgTextBounds(ctx, 0, 0, icon.data(), nullptr, nullptr);
              Vector2f iconPos(0, mPos.y() + m_size.y() * 0.5f - 1);

              if (mPopup->side() == Popup::Right)
                iconPos[0] = mPos.x() + m_size.x() - iw - 8;
              else
                iconPos[0] = mPos.x() + 8;

              nvgText(ctx, iconPos.x(), iconPos.y(), icon.data(), nullptr); */
        }
    }
}
