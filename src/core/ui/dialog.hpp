#pragma once

#include <string>
#include <vector>

#include "core/ds/color.hpp"
#include "core/ds/dimensions.hpp"
#include "core/ui/control.hpp"
#include "thirdparty/raygui.hpp"

namespace rl::ui
{
    class Dialog : public Control
    {
    public:
        using Control::Control;

        void setup() override
        {
            Control::setup();
            m_type = Control::Type::Dialog;
        }

        void draw() override
        {
            raylib::GuiWindowBox(m_rect, m_text.c_str());
        }

        constexpr virtual bool check_collision(ds::point<int32_t> pos) override
        {
            ds::rect<int32_t> status_bar{ m_rect };
            status_bar.height(RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT);
            if (status_bar.intersects(pos))
                return true;

            return false;
        }

        void teardown() override
        {
            Control::teardown();
        }
    };
}
