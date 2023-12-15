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
            return mValue;
        }

        void setValue(float value);

        Vector2i preferredSize(SDL3::SDL_Renderer* ctx) const override;
        void draw(SDL3::SDL_Renderer* renderer) override;
        void drawBody(SDL3::SDL_Renderer* renderer);
        void drawBar(SDL3::SDL_Renderer* renderer);

    protected:
        float mValue;

        struct AsyncTexture;
        typedef std::shared_ptr<AsyncTexture> AsyncTexturePtr;
        AsyncTexturePtr _body;
        AsyncTexturePtr _bar;
    };
}
