#pragma once

#include <memory>
#include <vector>

#include "ui/crtp/crtp_label.hpp"

namespace rl::test {
    void test_crtp_ui()
    {
        using namespace rl::nvg;
        using namespace rl::ui::crtp;

        auto root = std::make_shared<label>(widget::null(), "root widget");
        auto panel = std::make_shared<widget>(root, "base panel");
        auto label1 = std::make_shared<label>(panel, "label1");
        auto label2 = std::make_shared<label>(panel, "label2");
        auto label3 = std::make_shared<label>(panel, "label3");

        root->draw("drawing root node");

        std::vector<std::shared_ptr<widget>> widgets = {};
        widgets.push_back(root);
        widgets.push_back(panel);
        widgets.push_back(label1);
        widgets.push_back(label2);
        widgets.push_back(label3);
    }
}

namespace rl::test2 {
    struct EventHandler
    {
        void handle_event(this auto&& self, const std::string& text)
        {
            self.handle_event_impl(text);
        }
    };

    struct MouseEvent : EventHandler
    {
    public:
        explicit MouseEvent(std::string data)
            : m_mouse_event_member(std::move(data))
        {
        }

    private:
        friend EventHandler;

        void handle_event_impl(const std::string& text) const
        {
            std::print("[{}] MouseEvent event: {}\n", m_mouse_event_member, text);
        }

    private:
        std::string m_mouse_event_member{};
    };

    struct KeyboardEvent : EventHandler
    {
    public:
        explicit KeyboardEvent(std::string data)
            : m_kb_event_member(std::move(data))
        {
        }

    private:
        friend EventHandler;

        void handle_event_impl(const std::string& text) const
        {
            std::print("handling KeyboardEvent: {}\n", text);
        }

    private:
        std::string m_kb_event_member{};
    };

    inline void crtp_test()
    {
        using variant_t = std::variant<MouseEvent, KeyboardEvent>;

        variant_t mh1{ MouseEvent{ "mouse handler 1" } };
        variant_t mh2{ MouseEvent{ "mouse handler 2" } };
        variant_t mh3{ MouseEvent{ "mouse handler 3" } };
        variant_t kbh1{ KeyboardEvent{ "kb handler 1" } };
        variant_t kbh2{ KeyboardEvent{ "kb handler 2" } };
        variant_t kbh3{ KeyboardEvent{ "kb handler 3" } };
        variant_t kbh4{ KeyboardEvent{ "kb handler 4" } };

        std::vector<variant_t> event_handlers = {};
        event_handlers.push_back(mh1);
        event_handlers.push_back(mh2);
        event_handlers.push_back(mh3);
        event_handlers.push_back(kbh1);
        event_handlers.push_back(kbh2);
        event_handlers.push_back(kbh3);
        event_handlers.push_back(kbh4);

        for (auto&& [idx, handler] : event_handlers | std::ranges::views::enumerate) {
            if (KeyboardEvent* kbe = std::get_if<KeyboardEvent>(&handler))
                kbe->handle_event(std::to_string(idx));
            else if (MouseEvent* me = std::get_if<MouseEvent>(&handler))
                me->handle_event(std::to_string(idx));
        }
    }
}
