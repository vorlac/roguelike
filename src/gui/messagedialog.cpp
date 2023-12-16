#include <array>

#include "gui/button.hpp"
#include "gui/entypo.hpp"
#include "gui/label.hpp"
#include "gui/layout.hpp"
#include "gui/messagedialog.hpp"

namespace rl::gui {
    MessageDialog::MessageDialog(Widget* parent, Type type, const std::string& title,
                                 const std::string& message, const std::string& buttonText,
                                 const std::string& altButtonText, bool altButton)
        : Window(parent, title)
    {
        set_layout(new BoxLayout(Orientation::Vertical, Alignment::Middle, 10, 10));
        set_modal(true);

        Widget* panel1 = new Widget(this);
        panel1->set_layout(new BoxLayout(Orientation::Horizontal, Alignment::Middle, 10, 15));
        int icon = 0;
        switch (type)
        {
            case Type::Information:
                icon = ENTYPO_ICON_CIRCLED_INFO;
                break;
            case Type::Question:
                icon = ENTYPO_ICON_CIRCLED_HELP;
                break;
            case Type::Warning:
                icon = ENTYPO_ICON_WARNING;
                break;
        }
        Label* iconLabel = new Label(panel1, std::string(utf8(icon).data()), "icons");
        iconLabel->set_font_size(50);
        mMessageLabel = new Label(panel1, message);
        Widget* panel2 = new Widget(this);
        panel2->set_layout(new BoxLayout(Orientation::Horizontal, Alignment::Middle, 0, 15));

        if (altButton)
        {
            Button* button = new Button(panel2, altButtonText, ENTYPO_ICON_CIRCLED_CROSS);
            button->set_callback([&] {
                if (m_callback)
                    m_callback(1);
                dispose();
            });
        }
        Button* button = new Button(panel2, buttonText, ENTYPO_ICON_CHECK);
        button->set_callback([&] {
            if (m_callback)
                m_callback(0);
            dispose();
        });
        center();
        request_focus();
    }
}
