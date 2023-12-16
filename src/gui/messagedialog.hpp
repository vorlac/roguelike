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
            set_callback(callback);
        }

        MessageDialog(Widget* parent, Type type, const std::string& title,
                      const std::string& message, const std::function<void(int)>& callback)
            : MessageDialog(parent, type, title, message)
        {
            set_callback(callback);
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
            return m_callback;
        }

        void set_callback(const std::function<void(int)>& callback)
        {
            m_callback = callback;
        }

        MessageDialog& with_callback(const std::function<void(int)>& callback)
        {
            set_callback(callback);
            return *this;
        }

    protected:
        std::function<void(int)> m_callback;
        Label* mMessageLabel;
    };
}
