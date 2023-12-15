#pragma once

#include <memory>
#include <vector>

#include "gui/widget.hpp"

namespace rl::gui {

    class CheckBox : public Widget
    {
    public:
        CheckBox(Widget* parent, const std::string& caption = "Untitled",
                 const std::function<void(bool)>& callback = std::function<void(bool)>());

        const std::string& caption() const
        {
            return mCaption;
        }

        void setCaption(const std::string& caption)
        {
            mCaption = caption;
        }

        const bool& checked() const
        {
            return mChecked;
        }

        void setChecked(const bool& checked)
        {
            mChecked = checked;
        }

        CheckBox& withChecked(bool value)
        {
            setChecked(value);
            return *this;
        }

        const bool& pushed() const
        {
            return mPushed;
        }

        void setPushed(const bool& pushed)
        {
            mPushed = pushed;
        }

        std::function<void(bool)> callback() const
        {
            return mCallback;
        }

        void setCallback(const std::function<void(bool)>& callback)
        {
            mCallback = callback;
        }

        virtual bool mouseButtonEvent(const Vector2i& p, int button, bool down,
                                      int modifiers) override;
        Vector2i preferredSize(SDL3::SDL_Renderer* ctx) const override;
        void draw(SDL3::SDL_Renderer* ctx) override;
        virtual void drawBody(SDL3::SDL_Renderer* renderer);

    protected:
        std::string mCaption{};
        bool mPushed{};
        bool mChecked{};

        Texture _captionTex;
        Texture _pointTex;

        std::function<void(bool)> mCallback;

        struct AsyncTexture;
        typedef std::shared_ptr<AsyncTexture> AsyncTexturePtr;
        std::vector<AsyncTexturePtr> _txs;

        AsyncTexturePtr current_texture_ = nullptr;

    private:
        void drawTexture(AsyncTexturePtr& texture, SDL3::SDL_Renderer* renderer);
    };

}
