#include <memory>
#include <string>
#include <type_traits>
#include <utility>

#include "core/application.hpp"
#include "core/ui/crtp_label.hpp"
#include "core/ui/crtp_widget.hpp"
#include "graphics/vg/nanovg.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL_main.h>
SDL_C_LIB_END

int SDL3::main(int argc, char** argv)
{
    using namespace rl::vg;
    using namespace rl::ui;

    auto root = std::make_shared<crtp::label>(std::shared_ptr<crtp::widget>{},
                                              "root widget (label)", "");
    auto panel = std::make_shared<crtp::widget>(root, "base panel");
    auto label1 = std::make_shared<crtp::label>(panel, "label1", "");
    auto label2 = std::make_shared<crtp::label>(panel, "label2", "");
    auto label3 = std::make_shared<crtp::label>(panel, "label3", "");

    root->draw("root node");

    std::vector<std::shared_ptr<crtp::widget>> widgets = {};
    widgets.push_back(root);
    widgets.push_back(panel);
    widgets.push_back(label1);
    widgets.push_back(label2);
    widgets.push_back(label3);

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
