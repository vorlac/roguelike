#pragma once

#include <chrono>
#include <iostream>
#include <memory>
#include <thread>
#include <tuple>
#include <vector>

namespace rl
{
    template <class... Durations, class DurationIn>
    std::tuple<Durations...> to_durations(DurationIn d)
    {
        std::tuple<Durations...> retval;
        using discard = int[];
        (void)discard{
            0, (void(((std::get<Durations>(retval) = std::chrono::duration_cast<Durations>(d)),
                      (d -= std::chrono::duration_cast<DurationIn>(std::get<Durations>(retval))))),
                0)...
        };
        return retval;
    }
}
