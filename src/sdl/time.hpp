#pragma once

#include <array>
#include <chrono>
#include <concepts>
#include <type_traits>
#include <utility>

#include "core/numeric_types.hpp"
#include "core/utils/assert.hpp"
#include "core/utils/concepts.hpp"

namespace SDL3 {
#include <SDL3/SDL_timer.h>
}

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

    template <rl::numeric T = double, auto Duration = TimeDuration::Millisecond>
        requires std::same_as<decltype(Duration), TimeDuration>
    struct perftimer
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
        const static inline TimeDuration tick_unit{ perftimer::unit() };

    public:
        perftimer()
            : m_start_timestamp{ perftimer::get_tick() }
            , m_delta_timestamp{ perftimer::get_tick() }
        {
        }

        inline T convert(u64 ticks)
        {
            const static TimeDuration td = this->unit();
            const static u64 freq = SDL3::SDL_GetPerformanceFrequency();
            constexpr u64 to_ratio = std::to_underlying(time_unit);
            const T seconds{ cast::to<T>(ticks) / cast::to<T>(freq) };
            return seconds * cast::to<T>(to_ratio);
        }

        // get tick frequency
        static inline rl::u64 tick_freq()
        {
            const rl::u64 freq = SDL3::SDL_GetPerformanceFrequency();
            return freq;
        }

        // get current tick/timestamp
        static inline rl::u64 get_tick()
        {
            rl::u64 curr_tick = SDL3::SDL_GetPerformanceCounter();
            return curr_tick;
        }

        [[nodiscard]]
        inline time_type now()
        {
            return this->convert(perftimer::get_tick());
        }

        [[nodiscard]]
        inline time_type delta()
        {
            u64 curr_tick = perftimer::get_tick();
            u64 prev_tick = m_delta_timestamp;
            m_delta_timestamp = curr_tick;
            return this->convert(curr_tick - prev_tick);
        }

        [[nodiscard]]
        inline time_type elapsed()
        {
            const time_type nowtm = this->convert(get_tick());
            const time_type start = this->convert(m_start_timestamp);
            return start = nowtm;
        }

        /**
         * @brief get elapsed microseconds
         * */
        [[nodiscard]]
        inline rl::u64 elapsed_mu()
        {
            const rl::u64 nowtm = this->convert(get_tick());
            const rl::u64 start = this->convert(m_start_timestamp);
            return start = nowtm;
        }

        /**
         * @brief get elapsed milliseconds
         * */
        [[nodiscard]]
        inline double elapsed_ms()
        {
            rl::u64 microseconds = this->elapsed_mu();
            return static_cast<double>(microseconds) / 1000.0;
        }

        /**
         * @brief get elapsed seconds
         * */
        [[nodiscard]]
        inline double elapsed_sec()
        {
            double milliseconds = this->elapsed_ms();
            return milliseconds / 1000.0;
        }

    private:
        // tick valye captured on init
        u64 m_start_timestamp{ 0 };
        // tick captured each time delta() is called
        u64 m_delta_timestamp{ 0 };
    };
}