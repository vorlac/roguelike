#pragma once

#include <memory>

#include "gui/widget.hpp"

namespace rl::gui {
    /**
     * \class ProgressBar progressbar.h sdl_gui/progressbar.h
     *
     * \brief Standard widget for visualizing progress.
     */
    class ProgressBar : public Widget
    {
    public:
        explicit ProgressBar(Widget* parent);

        float value() const
        {
            return m_value;
        }

        void set_value(float value);

        Vector2i preferred_size(SDL3::SDL_Renderer* ctx) const override;
        void draw(SDL3::SDL_Renderer* renderer) override;
        void draw_body(SDL3::SDL_Renderer* renderer);
        void drawBar(SDL3::SDL_Renderer* renderer);

    protected:
        struct AsyncTexture;
        using AsyncTexturePtr = std::shared_ptr<ProgressBar::AsyncTexture>;

        float m_value{ 0.0f };
        ProgressBar::AsyncTexturePtr m_body;
        ProgressBar::AsyncTexturePtr m_bar;
    };
}
