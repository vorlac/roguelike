#pragma once

#include <chrono>
#include <concepts>
#include <type_traits>
#include <utility>

#include "core/assert.hpp"
#include "utils/concepts.hpp"
#include "utils/numeric.hpp"
#include "utils/sdl_defs.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL_timer.h>
SDL_C_LIB_END

namespace rl::inline utils {
    // time duration units used to interpret
    // as well as convert between timer values
    enum class TimeDuration : u64 {
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

    template <rl::numeric T = f32, auto FixedStep = 240, auto VDuration = TimeDuration::Second>
        requires std::same_as<decltype(VDuration), TimeDuration>
    struct Timer
    {
        // gets time unit of a single tick
        static TimeDuration unit()
        {
            constexpr static u64 pico{ std::to_underlying(TimeDuration::Picosecond) };
            constexpr static u64 nano{ std::to_underlying(TimeDuration::Nanosecond) };
            constexpr static u64 micr{ std::to_underlying(TimeDuration::Microsecond) };
            constexpr static u64 mili{ std::to_underlying(TimeDuration::Millisecond) };

            const u64 freq{ SDL3::SDL_GetPerformanceFrequency() };

            if (pico < freq)
                return TimeDuration::Picosecond;
            if (nano < freq)
                return TimeDuration::Nanosecond;
            if (micr < freq)
                return TimeDuration::Microsecond;
            if (mili < freq)
                return TimeDuration::Millisecond;

            // TODO: is the freq always an exact ratio?
            debug_assert(false, "unknow perf timer tick frequency");
            return TimeDuration::Unknown;
        }

    public:
        // data type that will be output
        using time_type = T;
        // the unti that will be output by the timer
        constexpr static inline TimeDuration time_unit{ VDuration };
        // the timer internally always stores the
        // highest resolution of time point units
        static inline const TimeDuration tick_unit{ Timer::unit() };

    public:
        [[nodiscard]]
        constexpr T convert(const u64 timestamp_duration)
        {
            constexpr u64 to_ratio{ std::to_underlying(time_unit) };
            const f32 seconds{ static_cast<f32>(timestamp_duration) / m_perf_counter_freq };
            return static_cast<T>(seconds * to_ratio);
        }

        [[nodiscard]]
        static u64 timer_freq()
        {
            // get tick frequency
            return SDL3::SDL_GetPerformanceFrequency();
        }

        [[nodiscard]]
        static u64 now()
        {
            // get current performance counter tick
            return SDL3::SDL_GetPerformanceCounter();
        }

        [[nodiscard]]
        constexpr time_type delta()
        {
            const u64 curr_timestamp{ Timer::now() };
            const u64 prev_timestamp{ m_delta_timestamp };

            ++m_tick_count;
            m_delta_timestamp = curr_timestamp;
            return this->convert(curr_timestamp - prev_timestamp);
        }

        [[nodiscard]]
        constexpr time_type elapsed()
        {
            const u64 curr_timestamp{ Timer::now() };
            const u64 init_timestamp{ m_start_timestamp };
            return this->convert(curr_timestamp - init_timestamp);
        }

        [[nodiscard]]
        constexpr u64 tick_count() const
        {
            return m_tick_count;
        }

        void reset()
        {
            m_start_timestamp = Timer::now();
        }

        template <std::invocable TCallable>
        void tick(TCallable&& callable)
        {
            // Query the current time.
            m_elapsed_time = this->elapsed();
            m_delta_time = m_elapsed_time - m_prev_tick_time;
            m_prev_tick_time = m_elapsed_time;
            m_fps_cur_timer += m_delta_time;

            const u64 last_frame_count{ m_frame_count };
            if (m_delta_time > m_max_delta_time)
                m_delta_time = m_max_delta_time;

            if (m_fixed_timestep > 0) {
                // Fixed timestep update logic
                if (std::abs(m_delta_time - m_fixed_timestep) < 1.0f / 4000)
                    m_delta_time = m_fixed_timestep;

                m_leftover_ticks += m_delta_time;
                while (m_leftover_ticks >= m_fixed_timestep) {
                    m_elapsed_time = m_fixed_timestep;
                    m_tick_timer += m_fixed_timestep;
                    m_leftover_ticks -= m_fixed_timestep;
                    ++m_frame_count;

                    std::invoke(std::forward<TCallable>(callable));
                }
            }
            else {
                // Variable timestep update logic.
                m_elapsed_time = m_delta_time;
                m_tick_timer += m_delta_time;
                m_leftover_ticks = 0;
                ++m_frame_count;

                std::invoke(std::forward<TCallable>(callable));
            }

            // Track the current framerate.
            if (m_frame_count != last_frame_count)
                ++m_fps_cur_count;

            if (m_fps_cur_timer >= 1.0f) {
                m_fps_avg_count = m_fps_cur_count;
                m_fps_cur_count = 0;
                m_fps_cur_timer -= 1.0f;
            }
        }

    private:
        // the number of times tick() has been called
        u64 m_tick_count{ 0 };
        u64 m_frame_count{ 0 };
        // tick valye captured on init
        u64 m_start_timestamp{ Timer::now() };
        // tick captured each time delta() is called
        u64 m_delta_timestamp{ Timer::now() };

        // elapsed time (in seconds) since timer
        // was initialized or last reset
        f64 m_elapsed_time{ 0.0 };
        // elapsed time (in seconds) since timer
        // was initialized or last reset
        f64 m_delta_time{ 0.0 };
        f64 m_max_delta_time{ 1.0 };
        // time when tick was last called (in seconds)
        f64 m_prev_tick_time{ 0.0 };
        f64 m_leftover_ticks{ 0.0 };
        f64 m_tick_timer{ 0.0 };

        f64 m_fixed_timestep{ FixedStep > 0 ? 1.0f / FixedStep : -1.0f };
        // total average ticks per second
        f64 m_fps_avg_count{ 0.0 };
        // ticks per second in the past second
        f64 m_fps_cur_count{ 0.0 };
        f64 m_fps_cur_timer{ 0.0 };

        constexpr static TimeDuration m_duration_unit{ VDuration };
        const u64 m_perf_counter_freq{ SDL3::SDL_GetPerformanceFrequency() };
    };
}
