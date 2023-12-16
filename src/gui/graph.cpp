#include <thread>

#include "gui/graph.hpp"
#include "gui/nanovg.h"
#include "gui/theme.hpp"

#define NANOVG_RT_IMPLEMENTATION
#define NANORT_IMPLEMENTATION
#include "gui/nanovg_rt.h"

#pragma warning(disable : 4838)

namespace rl::gui {
    struct Graph::AsyncTexture
    {
        Texture tex;
        NVGcontext* ctx = nullptr;

        void load(Graph* ptr)
        {
            Graph* graph = ptr;
            AsyncTexture* self = this;

            std::thread tgr([=]() {
                Theme* theme = graph->theme();

                int ww = graph->width();
                int hh = graph->height();
                NVGcontext* ctx = nvgCreateRT(NVG_DEBUG, ww, hh, 0);

                float pxRatio = 1.0f;
                nvgBeginFrame(ctx, ww, hh, pxRatio);

                nvgBeginPath(ctx);
                nvgRect(ctx, 0, 0, ww, hh);
                nvgFillColor(ctx, graph->background_color().to_nvg_color());
                nvgFill(ctx);

                if (graph->values().size() < 2)
                    return;

                nvgBeginPath(ctx);
                nvgMoveTo(ctx, 0, 0 + hh);
                auto& values = graph->values();
                for (size_t i = 0; i < (size_t)values.size(); i++)
                {
                    float value = values[i];
                    float vx = 0 + i * ww / (float)(values.size() - 1);
                    float vy = 0 + (1 - value) * hh;
                    nvgLineTo(ctx, vx, vy);
                }

                nvgLineTo(ctx, 0 + ww, 0 + hh);
                nvgStrokeColor(ctx, Color(100, 255).to_nvg_color());
                nvgStroke(ctx);
                nvgFillColor(ctx, graph->foregroundColor().to_nvg_color());
                nvgFill(ctx);

                nvgEndFrame(ctx);

                self->tex.rrect = { 0, 0, ww, hh };
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

    Graph::Graph(Widget* parent, const std::string& caption)
        : Widget(parent)
        , m_caption(caption)
    {
        m_background_color = Color(20, 128);
        mForegroundColor = Color(255, 192, 0, 128);
        m_text_color = Color(240, 192);
        m_caption_texture.dirty = true;
        _headerTex.dirty = true;
    }

    Vector2i Graph::preferred_size(SDL3::SDL_Renderer*) const
    {
        return Vector2i(180, 45);
    }

    void Graph::draw(SDL3::SDL_Renderer* renderer)
    {
        Widget::draw(renderer);

        auto&& ap = absolute_position();

        if (_atx)
        {
            _atx->perform(renderer);
            SDL3::SDL_FRect posRect{ ap.x, ap.y };
            SDL3::SDL_RenderTexture(renderer, _atx->tex.tex, &posRect, nullptr);
        }
        else
        {
            _atx = std::make_shared<AsyncTexture>();
            _atx->load(this);
        }

        if (m_caption_texture.dirty)
            m_theme->get_texture_and_rect_utf8(renderer, m_caption_texture, 0, 0, m_caption.c_str(),
                                               "sans", 14, m_text_color);

        if (_headerTex.dirty)
            m_theme->get_texture_and_rect_utf8(renderer, _headerTex, 0, 0, m_tab_header.c_str(),
                                               "sans", 18, m_text_color);

        if (_footerTex.dirty)
            m_theme->get_texture_and_rect_utf8(renderer, _footerTex, 0, 0, mFooter.c_str(), "sans",
                                               15, m_text_color);

        auto&& captionPos = ap + Vector2i(3, 1);
        SDL3::SDL_FRect posRectCaption{ captionPos.x, captionPos.y };

        auto&& headerPos = ap + Vector2i(m_size.x - 3 - _headerTex.w(), 1);
        SDL3::SDL_FRect posRectHeader{ headerPos.x, headerPos.y };

        auto&& footPos = ap +
                         Vector2i(m_size.x - 3 - _footerTex.w(), m_size.y - 1 - _footerTex.h());
        SDL3::SDL_FRect posRectFooter{ footPos.x, footPos.y };

        SDL3::SDL_RenderTexture(renderer, m_caption_texture.tex, &posRectCaption, nullptr);
        SDL3::SDL_RenderTexture(renderer, _headerTex.tex, &posRectHeader, nullptr);
        SDL3::SDL_RenderTexture(renderer, _footerTex.tex, &posRectFooter, nullptr);
    }
}
