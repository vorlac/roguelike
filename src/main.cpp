#include <memory>
#include <string>
#include <type_traits>
#include <utility>

#include "core/application.hpp"
#include "core/ui/crtp/crtp_label.hpp"
#include "core/ui/crtp/crtp_widget.hpp"
#include "graphics/vg/nanovg.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL_main.h>
SDL_C_LIB_END

void test_crtp_ui()
{
    using namespace rl::vg;
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

int SDL3::main(int argc, char** argv)
{
    int ret = -1;
    if (!rl::parse_args(argc, argv))
        return 1;
    else
    {
        rl::Application game{};
        ret = game.run();
    }

    SDL3::SDL_Quit();
    return ret;
}
