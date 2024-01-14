#pragma once

#include <memory>
#include <string>

#include "core/ui/crtp/crtp_widget.hpp"
#include "graphics/vg/nanovg.hpp"
#include "utils/numeric.hpp"

namespace rl::ui::crtp {

    struct label : public widget
    {
    public:
        template <typename TWidget>
        constexpr inline explicit label(const std::shared_ptr<TWidget>& parent,
                                        const std::string& widget_name = "",
                                        const std::string& label_text = "")
            : crtp::widget{ std::forward<decltype(parent)>(parent),
                            std::forward<decltype(widget_name)>(widget_name) }
            , m_text{ std::forward<decltype(label_text)>(label_text) }
        {
        }

        constexpr inline std::string label_text()
        {
            return m_text;
        }

    private:
        friend control;
        friend widget;

        inline void draw_impl(const std::string& text)
        {
            for (auto&& child : this->children())
            {
                const auto& child_name{ child->name() };
                child->draw(child_name);
            }
        }

    private:
        std::string m_text{};
    };
}
