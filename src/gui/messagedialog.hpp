#pragma once

#include "gui/window.hpp"

namespace rl::gui {
    class Label;

    class MessageDialog : public Window
    {
    public:
        enum class Type {
            Information,
            Question,
            Warning
        };

        MessageDialog(Widget* parent, Type type, const std::string& title = "Untitled",
                      const std::string& message = "Message", const std::string& buttonText = "OK",
                      const std::string& altButtonText = "Cancel", bool altButton = false);

        MessageDialog(Widget* parent, Type type, const std::string& title,
                      const std::string& message, const std::string& buttonText,
                      const std::string& altButtonText, bool altButton,
                      const std::function<void(int)>& callback)
            : MessageDialog(parent, type, title, message, buttonText, altButtonText, altButton)
        {
            setCallback(callback);
        }

        MessageDialog(Widget* parent, Type type, const std::string& title,
                      const std::string& message, const std::function<void(int)>& callback)
            : MessageDialog(parent, type, title, message)
        {
            setCallback(callback);
        }

        Label* messageLabel()
        {
            return mMessageLabel;
        }

        const Label* messageLabel() const
        {
            return mMessageLabel;
        }

        std::function<void(int)> callback() const
        {
            return mCallback;
        }

        void setCallback(const std::function<void(int)>& callback)
        {
            mCallback = callback;
        }

        MessageDialog& withCallback(const std::function<void(int)>& callback)
        {
            setCallback(callback);
            return *this;
        }

    protected:
        std::function<void(int)> mCallback;
        Label* mMessageLabel;
    };
}
