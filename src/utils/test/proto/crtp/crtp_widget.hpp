#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "ui/crtp/crtp_control.hpp"
#include "utils/io.hpp"

namespace rl::ui::crtp {
    struct label;

    struct widget : public control
    {
    public:
        template <typename TWidget>
        constexpr inline explicit widget(const std::shared_ptr<TWidget>& parent,
                                         const std::string& widget_name)
            : m_parent{ std::forward<decltype(parent)>(parent) }
            , m_name{ std::forward<decltype(widget_name)>(widget_name) }
        {
            if (parent != nullptr)
                parent->add_child(std::shared_ptr<widget>{ this });
        }

        static inline const std::shared_ptr<widget> null()
        {
            return std::shared_ptr<widget>{ nullptr };
        }

        constexpr inline std::string name()
        {
            return m_name;
        }

        constexpr inline std::vector<std::shared_ptr<crtp::widget>>& children()
        {
            return m_children;
        }

    private:
        friend control;
        friend label;

        inline void draw_impl(const std::string& text)
        {
            for (auto&& child : m_children) {
                const auto& child_name{ child->name() };
                child->draw(child_name);
            }
        }

        template <typename TWidget>
        inline void add_child_impl(std::shared_ptr<TWidget> child)
        {
            m_children.push_back(child);
        }

    private:
        std::shared_ptr<crtp::widget> m_parent{};
        std::vector<std::shared_ptr<crtp::widget>> m_children{};
        std::string m_name{};
    };
}
