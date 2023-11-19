#pragma once

#include <array>
#include <chrono>
#include <concepts>
#include <type_traits>
#include <utility>

#include <fmt/chrono.h>
#include <fmt/core.h>
#include <fmt/format.h>

#include "core/numeric_types.hpp"
#include "sdl/defs.hpp"
#include "utils/assert.hpp"
#include "utils/concepts.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL_timer.h>
SDL_C_LIB_END

namespace rl::sdl {
    // time duration units used to interpret
    // as well as convert between timer values
    enum class TimeDuration : rl::u64 {
        // clang-format off
        Unknown      = 0,
        Second       = 1,
        Decisecond   = 10,
        Centisecond  = 100,
        Millisecond  = 1000,
        Microsecond  = 1000000,
        Nanosecond   = 1000000000,
        Picosecond   = 1000000000000,
        Femtosecond  = 1000000000000000,
        // clang-format on
    };

    template <rl::numeric T = double, auto Duration = TimeDuration::Second>
        requires std::same_as<decltype(Duration), TimeDuration>
    struct Timer
    {
        // gets time unit of a single tick
        static inline TimeDuration unit()
        {
            u64 freq = SDL3::SDL_GetPerformanceFrequency();

            constexpr u64 pico = std::to_underlying(TimeDuration::Picosecond);
            if (pico < freq)
                return TimeDuration::Picosecond;

            constexpr u64 nano = std::to_underlying(TimeDuration::Nanosecond);
            if (nano < freq)
                return TimeDuration::Nanosecond;

            constexpr u64 micr = std::to_underlying(TimeDuration::Microsecond);
            if (micr < freq)
                return TimeDuration::Microsecond;

            constexpr u64 mili = std::to_underlying(TimeDuration::Millisecond);
            if (mili < freq)
                return TimeDuration::Millisecond;

            // TODO: is the freq always an exact ratio?
            runtime_assert(false, "unknow perf timer tick frequency");
            return TimeDuration::Unknown;
        }

    public:
        // data type that will be output
        using time_type = T;
        // the unti that will be output by the timer
        constexpr static inline TimeDuration time_unit{ Duration };
        // the timer internally always stores the
        // highest resolution of time point units
        const static inline TimeDuration tick_unit{ Timer::unit() };

    public:
        constexpr inline Timer()
            : m_start_timestamp{ Timer::get_tick() }
            , m_delta_timestamp{ Timer::get_tick() }
        {
        }

        constexpr inline T convert(u64 ticks)
        {
            const static TimeDuration td = this->unit();
            const static u64 freq = SDL3::SDL_GetPerformanceFrequency();
            constexpr u64 to_ratio = std::to_underlying(time_unit);
            const double seconds{ ticks / static_cast<double>(freq) };
            return static_cast<T>(seconds * to_ratio);
        }

        // get tick frequency
        static inline u64 tick_freq()
        {
            const u64 freq = SDL3::SDL_GetPerformanceFrequency();
            return freq;
        }

        // get current tick/timestamp
        static inline u64 get_tick()
        {
            u64 curr_tick = SDL3::SDL_GetPerformanceCounter();
            return curr_tick;
        }

        [[nodiscard]]
        constexpr inline time_type now()
        {
            return this->convert(Timer::get_tick());
        }

        [[nodiscard]]
        inline time_type delta()
        {
            const u64 curr_tick = Timer::get_tick();
            const u64 prev_tick = m_delta_timestamp;
            m_delta_timestamp = curr_tick;
            return this->convert(curr_tick - prev_tick);
        }

        [[nodiscard]]
        inline time_type elapsed()
        {
            const u64 curr_tick = Timer::get_tick();
            const u64 prev_tick = m_start_timestamp;
            return this->convert(curr_tick - prev_tick);
        }

    private:
        // tick valye captured on init
        u64 m_start_timestamp{ 0 };
        // tick captured each time delta() is called
        u64 m_delta_timestamp{ 0 };
    };
}
