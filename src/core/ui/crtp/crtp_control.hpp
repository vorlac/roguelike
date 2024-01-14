#pragma once

#include <memory>
#include <string>

#include "utils/io.hpp"

namespace rl::ui::crtp {
    struct widget;

    struct control
    {
        inline void draw(this auto&& self, const std::string& text)
        {
            self.draw_impl(text);
        }

        template <typename TWidget>
        inline void add_child(this auto&& self, std::shared_ptr<TWidget> child)
        {
            if constexpr (std::same_as<TWidget, crtp::widget>)
                self.add_child_impl(std::forward<std::shared_ptr<TWidget>>(child));
            else
            {
                auto& w{ static_cast<crtp::widget*>(&self) };
                w->add_child(std::forward<std::shared_ptr<TWidget>>(child));
            }
        }
    };
}
