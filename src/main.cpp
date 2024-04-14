#include <ranges>
#include <type_traits>

#include "core/application.hpp"
#include "ds/rect.hpp"
#include "utils/options.hpp"

int main(const int argc, char** argv)
{
    using namespace rl;
    constexpr static ds::rect<f32> rectangles[]{
        ds::rect<f32>{
            .pt = ds::point<f32>{ .x = 1.0f, .y = 2.0f },
            .size = ds::dims<f32>{ .width = 10.0f, .height = 10.0f },
        },
        ds::rect<f32>{
            .pt = ds::point<f32>{ .x = 3.0f, .y = 4.0f },
            .size = ds::dims<f32>{ .width = 10.0f, .height = 10.0f },
        },
        ds::rect<f32>{
            .pt = ds::point<f32>{ .x = 5.0f, .y = 6.0f },
            .size = ds::dims<f32>{ .width = 10.0f, .height = 10.0f },
        },
        ds::rect<f32>{
            .pt = ds::point<f32>{ .x = 7.0f, .y = 8.0f },
            .size = ds::dims<f32>{ .width = 10.0f, .height = 10.0f },
        },
    };

    const ds::rect<f32>* rects{ rectangles };
    int32_t size{ sizeof(rectangles) / sizeof(rectangles[0]) };

    for (auto i : std::ranges::views::iota(0, size))
    {
        const ds::rect<f32>& curr_rect = rects[i];
        fmt::print("{}) {}\n", i, curr_rect);
    }

    int ret = -1;
    if (!rl::parse_args(argc, argv))
        ret = 1;
    else
    {
        rl::Application game{};
        ret = game.run();
    }

    return ret;
}
