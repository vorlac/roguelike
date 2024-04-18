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
