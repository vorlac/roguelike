#pragma once

#include <functional>
#include <string>

#include "core/ui/widget.hpp"
#include "ds/dims.hpp"

namespace rl {
    class Mouse;
    class Keyboard;

    namespace vg {
        struct NVGcontext;
    }
}

namespace rl::ui {

    class CheckBox : public ui::Widget
    {
    public:
        CheckBox(ui::Widget* parent, const std::string& caption = "UntitledCB",
                 const std::function<void(bool)>& callback = std::function<void(bool)>());

        const bool& checked() const;
        const bool& pushed() const;
        const std::string& caption() const;

        void set_checked(const bool& checked);
        void set_pushed(const bool& pushed);
        void set_caption(const std::string& caption);

        const std::function<void(bool)>& callback() const;
        void set_callback(const std::function<void(bool)>& toggled_callback);

    public:
        virtual bool on_mouse_button_pressed(const Mouse& mouse, const Keyboard& kb) override;
        virtual bool on_mouse_button_released(const Mouse& mouse, const Keyboard& kb) override;

        virtual ds::dims<f32> preferred_size(vg::NVGcontext* nvg_context) const override;
        virtual void draw(vg::NVGcontext* nvg_context) override;

    protected:
        bool m_pushed{ false };
        bool m_checked{ false };
        std::string m_caption{ "" };
        std::function<void(bool)> m_toggled_callback;
    };
}
