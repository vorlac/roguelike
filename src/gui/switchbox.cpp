#include <thread>

#include "gui/nanovg.h"
#include "gui/switchbox.hpp"
#include "gui/theme.hpp"

#define NANOVG_RT_IMPLEMENTATION
#define NANORT_IMPLEMENTATION
#include "gui/nanovg_rt.h"

namespace rl::gui {
    struct SwitchBox::AsyncTexture
    {
        int id;
        Texture tex;
        NVGcontext* ctx = nullptr;

        AsyncTexture(int tex_id)
            : id(tex_id)
        {
        }

        void load_body(SwitchBox* ptr, bool enabled)
        {
            SwitchBox* sb = ptr;
            AsyncTexture* self = this;
            std::thread tgr([=]() {
                Theme* theme = sb->theme();

                int ww = sb->width();
                int hh = sb->height();
                NVGcontext* ctx = nvgCreateRT(NVG_DEBUG, ww, hh, 0);

                float pxRatio = 1.0f;
                nvgBeginFrame(ctx, ww, hh, pxRatio);

                Vector2f center = sb->size().cast<float>() * 0.5f;
                float kr, startX, startY, widthX, heightY;
                if (sb->m_align == Alignment::Horizontal)
                {
                    kr = hh * 0.4f;
                    startX = hh * 0.1f;
                    heightY = hh * 0.8;

                    startY = ((hh - heightY) / 2) + 1;
                    widthX = (hh * 1.5);
                }
                else
                {
                    kr = hh * 0.2f;
                    startX = hh * 0.05f + 1;
                    heightY = hh * 0.8;

                    startY = ((hh - heightY) / 2);
                    widthX = (hh * 0.4f);
                }

                NVGpaint bg = nvgBoxGradient(ctx, startX, startY, widthX, heightY, 3, 3,
                                             Color(0, enabled ? 32 : 10).to_nvg_color(),
                                             Color(0, enabled ? 128 : 210).to_nvg_color());

                nvgBeginPath(ctx);
                nvgRoundedRect(ctx, startX, startY, widthX, heightY, kr);
                nvgFillPaint(ctx, bg);

                nvgBeginPath(ctx);
                nvgStrokeWidth(ctx, 1.0f);
                nvgRoundedRect(ctx, startX + 0.5f, startY + 0.5f, widthX - 1, heightY - 1, kr);
                nvgStrokeColor(ctx, theme->m_border_light.to_nvg_color());
                nvgStroke(ctx);
                nvgFill(ctx);

                nvgBeginPath(ctx);
                nvgRoundedRect(ctx, startX + 0.5f, startY + 0.5f, widthX - 1, heightY - 2, kr);
                nvgStrokeColor(ctx, theme->m_border_dark.to_nvg_color());
                nvgStroke(ctx);

                nvgEndFrame(ctx);

                self->tex.rrect = { 0, 0, ww, hh };
                self->ctx = ctx;
            });

            tgr.detach();
        }

        void load_knob(SwitchBox* ptr, bool enabled)
        {
            SwitchBox* sb = ptr;
            AsyncTexture* self = this;
            std::thread tgr([=]() {
                Theme* theme = sb->theme();

                int ww = std::min(sb->width(), sb->height());
                int hh = ww;

                Vector2f center(ww / 2, hh / 2);
                float kr = hh * 0.4f;

                NVGcontext* ctx = nvgCreateRT(NVG_DEBUG, ww, ww, 0);

                float pxRatio = 1.0f;
                nvgBeginFrame(ctx, ww, ww, pxRatio);

                NVGpaint knob = nvgLinearGradient(ctx, 0, center.y - kr, 0, center.y + kr,
                                                  theme->m_border_light.to_nvg_color(),
                                                  theme->m_border_medium.to_nvg_color());
                NVGpaint knobReverse = nvgLinearGradient(ctx, 0, center.y - kr, 0, center.y + kr,
                                                         theme->m_border_medium.to_nvg_color(),
                                                         theme->m_border_light.to_nvg_color());

                nvgBeginPath(ctx);
                nvgCircle(ctx, center.x, center.y, kr * 0.9);
                nvgStrokeColor(ctx, Color(0, 200).to_nvg_color());
                nvgFillPaint(ctx, knob);
                nvgStroke(ctx);
                nvgFill(ctx);
                nvgBeginPath(ctx);
                nvgCircle(ctx, center.x, center.y, kr * 0.7);
                nvgFillColor(ctx, Color(120, enabled ? 255 : 100).to_nvg_color());
                nvgStrokePaint(ctx, knobReverse);
                nvgStroke(ctx);
                nvgFill(ctx);

                nvgEndFrame(ctx);

                self->tex.rrect = { 0, 0, ww, ww };
                self->ctx = ctx;
            });

            tgr.detach();
        }

        void perform(SDL3::SDL_Renderer* renderer)
        {
            if (!ctx)
                return;

            unsigned char* rgba = nvgReadPixelsRT(ctx);

            tex.tex = SDL3::SDL_CreateTexture(renderer, SDL3::SDL_PIXELFORMAT_ABGR8888,
                                              SDL3::SDL_TEXTUREACCESS_STREAMING, tex.w(), tex.h());

            int pitch;
            uint8_t* pixels;
            int ok = SDL3::SDL_LockTexture(tex.tex, nullptr, (void**)&pixels, &pitch);
            memcpy(pixels, rgba, sizeof(uint32_t) * tex.w() * tex.h());
            SDL3::SDL_SetTextureBlendMode(tex.tex, SDL3::SDL_BLENDMODE_BLEND);
            SDL3::SDL_UnlockTexture(tex.tex);

            nvgDeleteRT(ctx);
            ctx = nullptr;
        }
    };

    SwitchBox::SwitchBox(Widget* parent, Alignment align, const std::string& caption,
                         const std::function<void(bool)>& callback)
        : CheckBox(parent, caption, callback)
        , m_align(align)
    {
    }

    Vector2i SwitchBox::preferred_size(SDL3::SDL_Renderer* renderer) const
    {
        if (m_fixed_size != Vector2i::zero())
            return m_fixed_size;

        int w, h;
        const_cast<SwitchBox*>(this)->theme()->get_utf8_bounds("sans", font_size(),
                                                               m_caption.c_str(), &w, &h);
        int knobW = 1.8f * font_size();
        knobW = std::max<int>(knobW / 32, 1) * 32;

        if (m_align == Alignment::Horizontal)
            return Vector2i(w + knobW, knobW);
        else
            return Vector2i(w + knobW, 2 * knobW);
    }

    void SwitchBox::draw_body(SDL3::SDL_Renderer* renderer)
    {
        int id = (0x100) + (m_enabled ? 1 : 0);
        auto atx = std::find_if(m_textures.begin(), m_textures.end(),
                                [id](SwitchBox::AsyncTexturePtr p) {
                                    return p->id == id;
                                });

        if (atx != m_textures.end())
        {
            Vector2i ap = absolute_position();
            (*atx)->perform(renderer);
            SDL_RenderCopy(renderer, (*atx)->tex, ap);
        }
        else
        {
            SwitchBox::AsyncTexturePtr newtx = std::make_shared<SwitchBox::AsyncTexture>(id);
            newtx->load_body(this, m_enabled);
            m_textures.push_back(newtx);
        }
    }

    void SwitchBox::drawKnob(SDL3::SDL_Renderer* renderer)
    {
        int id = (0x200) + (m_enabled ? 1 : 0);
        auto atx = std::find_if(m_textures.begin(), m_textures.end(),
                                [id](SwitchBox::AsyncTexturePtr p) {
                                    return p->id == id;
                                });

        Vector2i ap = absolute_position();
        Vector2f center = ap.as<float>() + m_size.as<float>() * 0.5f;
        Vector2i knobPos;
        float kr, startX, startY, widthX, heightY, hh;
        hh = height();
        if (m_align == Alignment::Horizontal)
        {
            kr = (hh * 0.4f);
            startX = ap.x + hh * 0.1f;
            heightY = hh * 0.8;

            startY = (ap.y + (hh - heightY) / 2) + 1;
            widthX = (hh * 1.5);

            knobPos = Vector2i(startX + kr + m_path * (widthX - 2 * kr), center.y + 0.5f);
        }
        else
        {
            kr = (hh * 0.2f);
            startX = ap.x + hh * 0.05f + 1;
            heightY = hh * 0.8;

            startY = (ap.y + (hh - heightY) / 2);
            widthX = (hh * 0.4f);

            knobPos = Vector2i(startX + kr, startY + m_path * (heightY - 2 * kr) + kr);
        }

        if (atx != m_textures.end())
        {
            (*atx)->perform(renderer);
            SDL_RenderCopy(renderer, (*atx)->tex,
                           knobPos - Vector2i((*atx)->tex.w() / 2, (*atx)->tex.h() / 2));
        }
        else
        {
            SwitchBox::AsyncTexturePtr newtx = std::make_shared<SwitchBox::AsyncTexture>(id);
            newtx->load_knob(this, m_enabled);
            m_textures.push_back(newtx);
        }
    }

    void SwitchBox::draw(SDL3::SDL_Renderer* renderer)
    {
        if (m_checked)
        {
            if (m_path < 1.0f)
                m_path += 0.1f;
        }
        else
        {
            if (m_path > 0)
                m_path -= 0.1f;
            if (m_path < 0)
                m_path = 0;
        }

        draw_body(renderer);
        drawKnob(renderer);

        // nvgFontSize(ctx, font_size());
        // nvgFontFace(ctx, "sans");
        // nvgFillColor(ctx, mEnabled ? mTheme->m_text_color : mTheme->m_disabled_text_color);
        // nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
        // nvgText(ctx, mPos.x() + 1.6f * font_size(), mPos.y() + m_size.y() * 0.5f,
        // m_caption.c_str(),
        //         nullptr);

        Widget::draw(renderer);
    }
}
