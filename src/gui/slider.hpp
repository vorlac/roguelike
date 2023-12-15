#pragma once

#include <memory>

#include "gui/widget.hpp"

namespace rl::gui {

    /**
     * \class Slider slider.h sdl_gui/slider.h
     *
     * \brief Fractional slider widget with mouse control.
     */
    class Slider : public Widget
    {
    public:
        Slider(Widget* parent, float value = 0.f);

        Slider(Widget* parent, float value, const std::function<void(Slider*, float)>& objcallback)
            : Slider(parent, value)
        {
            setObjCallback(objcallback);
        }

        Slider(Widget* parent, float value, const std::function<void(Slider*, float)>& objcallback,
               const std::function<void(float)>& fcallback)
            : Slider(parent, value)
        {
            setObjCallback(objcallback);
            setFinalCallback(fcallback);
        }

        float value() const
        {
            return mValue;
        }

        void setValue(float value)
        {
            mValue = value;
        }

        const Color& highlightColor() const
        {
            return mHighlightColor;
        }

        void setHighlightColor(const Color& highlightColor)
        {
            mHighlightColor = highlightColor;
        }

        std::pair<float, float> highlightedRange() const
        {
            return mHighlightedRange;
        }

        void setHighlightedRange(std::pair<float, float> highlightedRange)
        {
            mHighlightedRange = highlightedRange;
        }

        std::pair<float, float> range() const
        {
            return mRange;
        }

        void setRange(std::pair<float, float> range)
        {
            mRange = range;
        }

        std::function<void(float)> callback() const
        {
            return mCallback;
        }

        void setCallback(const std::function<void(float)>& callback)
        {
            mCallback = callback;
        }

        std::function<void(Slider*, float)> objcallback() const
        {
            return mObjCallback;
        }

        void setObjCallback(const std::function<void(Slider*, float)>& callback)
        {
            mObjCallback = callback;
        }

        std::function<void(float)> finalCallback() const
        {
            return mFinalCallback;
        }

        void setFinalCallback(const std::function<void(float)>& callback)
        {
            mFinalCallback = callback;
        }

        Vector2i preferredSize(SDL3::SDL_Renderer* ctx) const override;
        bool mouseDragEvent(const Vector2i& p, const Vector2i& rel, int button,
                            int modifiers) override;
        bool mouseButtonEvent(const Vector2i& p, int button, bool down, int modifiers) override;
        void draw(SDL3::SDL_Renderer* renderer) override;

        virtual void drawBody(SDL3::SDL_Renderer* renderer);
        virtual void drawKnob(SDL3::SDL_Renderer* renderer);

        void setKnobOutterRadiusKoeff(float koeff)
        {
            mKnobRadKoeff.outter = koeff;
        }

        void setKnobInnerRadiusKoeff(float koeff)
        {
            mKnobRadKoeff.inner = koeff;
        }

    protected:
        float mValue;
        bool _lastEnabledState = false;

        struct
        {
            float outter = 1.f;
            float inner = 0.5f;
        } mKnobRadKoeff;

        std::function<void(float)> mCallback;
        std::function<void(Slider*, float)> mObjCallback;
        std::function<void(float)> mFinalCallback;
        std::pair<float, float> mRange;
        std::pair<float, float> mHighlightedRange;
        Color mHighlightColor;

        struct AsyncTexture;
        typedef std::shared_ptr<AsyncTexture> AsyncTexturePtr;
        AsyncTexturePtr _body;
        AsyncTexturePtr _knob;
    };
}
