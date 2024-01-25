#pragma once

#include <array>
#include <chrono>
#include <concepts>
#include <type_traits>
#include <utility>

#include "core/assert.hpp"
#include "sdl/defs.hpp"
#include "utils/concepts.hpp"
#include "utils/numeric.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL_timer.h>
SDL_C_LIB_END

namespace rl::inline utils {
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
            constexpr static u64 pico = std::to_underlying(TimeDuration::Picosecond);
            constexpr static u64 nano = std::to_underlying(TimeDuration::Nanosecond);
            constexpr static u64 micr = std::to_underlying(TimeDuration::Microsecond);
            constexpr static u64 mili = std::to_underlying(TimeDuration::Millisecond);

            const u64 freq = SDL3::SDL_GetPerformanceFrequency();

            if (pico < freq)
                return TimeDuration::Picosecond;
            if (nano < freq)
                return TimeDuration::Nanosecond;
            if (micr < freq)
                return TimeDuration::Microsecond;
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
        static inline const TimeDuration tick_unit{ Timer::unit() };

    public:
        constexpr inline Timer()
            : m_start_timestamp{ Timer::get_tick() }
            , m_delta_timestamp{ Timer::get_tick() }
        {
        }

        constexpr inline T convert(u64 ticks)
        {
            static const TimeDuration td = this->unit();
            static const u64 freq = SDL3::SDL_GetPerformanceFrequency();
            constexpr u64 to_ratio = std::to_underlying(time_unit);
            const f64 seconds{ ticks / static_cast<f64>(freq) };
            return static_cast<T>(seconds * to_ratio);
        }

        // get tick frequency
        static inline u64 tick_freq()
        {
            return SDL3::SDL_GetPerformanceFrequency();
        }

        // get current tick/timestamp
        static inline u64 get_tick()
        {
            return SDL3::SDL_GetPerformanceCounter();
        }

        [[nodiscard]]
        constexpr inline time_type now() const
        {
            return this->convert(Timer::get_tick());
        }

        [[nodiscard]]
        inline time_type delta()
        {
            ++m_tick_count;

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

        [[nodiscard]]
        inline u64 tick_count() const
        {
            return m_tick_count;
        }

        inline void reset()
        {
            m_start_timestamp = Timer::get_tick();
        }

    private:
        u64 m_tick_count{ 0 };
        // tick valye captured on init
        u64 m_start_timestamp{ 0 };
        // tick captured each time delta() is called
        u64 m_delta_timestamp{ 0 };
    };

    // class StepTimer
    //{
    // public:
    //     StepTimer() noexcept(false)
    //         : m_elapsedTicks(0)
    //         , m_totalTicks(0)
    //         , m_leftOverTicks(0)
    //         , m_frameCount(0)
    //         , m_framesPerSecond(0)
    //         , m_framesThisSecond(0)
    //         , m_qpcSecondCounter(0)
    //         , m_isFixedTimeStep(false)
    //         , m_targetElapsedTicks(TicksPerSecond / 60)
    //     {
    //         if (!::QueryPerformanceFrequency(&m_qpcFrequency))
    //             throw std::exception("QueryPerformanceFrequency");

    //        if (!::QueryPerformanceCounter(&m_qpcLastTime))
    //            throw std::exception("QueryPerformanceCounter");

    //        // Initialize max delta to 1/10 of a second.
    //        m_qpcMaxDelta = static_cast<u64>(m_qpcFrequency.QuadPart / 10);
    //    }

    //    // Get elapsed time since the previous Update call.
    //    uint64_t GetElapsedTicks() const noexcept
    //    {
    //        return m_elapsedTicks;
    //    }

    //    double GetElapsedSeconds() const noexcept
    //    {
    //        return TicksToSeconds(m_elapsedTicks);
    //    }

    //    // Get total time since the start of the program.
    //    uint64_t GetTotalTicks() const noexcept
    //    {
    //        return m_totalTicks;
    //    }

    //    double GetTotalSeconds() const noexcept
    //    {
    //        return TicksToSeconds(m_totalTicks);
    //    }

    //    // Get total number of updates since start of the program.
    //    u64 GetFrameCount() const noexcept
    //    {
    //        return m_frameCount;
    //    }

    //    // Get the current framerate.
    //    u64 GetFramesPerSecond() const noexcept
    //    {
    //        return m_framesPerSecond;
    //    }

    //    // Set whether to use fixed or variable timestep mode.
    //    void SetFixedTimeStep(bool isFixedTimestep) noexcept
    //    {
    //        m_isFixedTimeStep = isFixedTimestep;
    //    }

    //    // Set how often to call Update when in fixed timestep mode.
    //    void SetTargetElapsedTicks(u64 targetElapsed) noexcept
    //    {
    //        m_targetElapsedTicks = targetElapsed;
    //    }

    //    void SetTargetElapsedSeconds(double targetElapsed) noexcept
    //    {
    //        m_targetElapsedTicks = SecondsToTicks(targetElapsed);
    //    }

    //    // Integer format represents time using 10,000,000 ticks per second.
    //    const static u64 TicksPerSecond = 10000000;

    //    constexpr static double TicksToSeconds(u64 ticks) noexcept
    //    {
    //        return static_cast<double>(ticks) / TicksPerSecond;
    //    }

    //    constexpr static u64 SecondsToTicks(double seconds) noexcept
    //    {
    //        return static_cast<u64>(seconds * TicksPerSecond);
    //    }

    //    // After an intentional timing discontinuity (for instance a blocking IO operation)
    //    // call this to avoid having the fixed timestep logic attempt a set of catch-up
    //    // Update calls.

    //    void ResetElapsedTime()
    //    {
    //        if (!QueryPerformanceCounter(&m_qpcLastTime))
    //            throw std::exception("QueryPerformanceCounter");

    //        m_leftOverTicks = 0;
    //        m_framesPerSecond = 0;
    //        m_framesThisSecond = 0;
    //        m_qpcSecondCounter = 0;
    //    }

    //    // Update timer state, calling the specified Update function the appropriate number of
    //    // times.
    //    template <typename TUpdate>
    //    void Tick(const TUpdate& update)
    //    {
    //        // Query the current time.
    //        LARGE_INTEGER currentTime;

    //        if (!QueryPerformanceCounter(&currentTime))
    //            throw std::exception("QueryPerformanceCounter");

    //        u64 timeDelta = static_cast<u64>(currentTime.QuadPart - m_qpcLastTime.QuadPart);

    //        m_qpcLastTime = currentTime;
    //        m_qpcSecondCounter += timeDelta;

    //        // Clamp excessively large time deltas (e.g. after paused in the debugger).
    //        if (timeDelta > m_qpcMaxDelta)
    //            timeDelta = m_qpcMaxDelta;

    //        // Convert QPC units into a canonical tick format. This cannot overflow due to the
    //        // previous clamp.
    //        timeDelta *= TicksPerSecond;
    //        timeDelta /= static_cast<u64>(m_qpcFrequency.QuadPart);

    //        u32 lastFrameCount = m_frameCount;

    //        if (m_isFixedTimeStep)
    //        {
    //            // Fixed timestep update logic

    //            // If the app is running very close to the target elapsed time (within 1/4 of a
    //            // millisecond) just clamp the clock to exactly match the target value. This
    //            // prevents tiny and irrelevant errors from accumulating over time. Without this
    //            // clamping, a game that requested a 60 fps fixed update, running with vsync
    //            enabled
    //            // on a 59.94 NTSC display, would eventually accumulate enough tiny errors that it
    //            // would drop a frame. It is better to just round small deviations down to zero to
    //            // leave things running smoothly.

    //            if (static_cast<u64>(std::abs(static_cast<u64>(timeDelta - m_targetElapsedTicks)))
    //            <
    //                TicksPerSecond / 4000)
    //            {
    //                timeDelta = m_targetElapsedTicks;
    //            }

    //            m_leftOverTicks += timeDelta;

    //            while (m_leftOverTicks >= m_targetElapsedTicks)
    //            {
    //                m_elapsedTicks = m_targetElapsedTicks;
    //                m_totalTicks += m_targetElapsedTicks;
    //                m_leftOverTicks -= m_targetElapsedTicks;
    //                m_frameCount++;

    //                update();
    //            }
    //        }
    //        else
    //        {
    //            // Variable timestep update logic.
    //            m_elapsedTicks = timeDelta;
    //            m_totalTicks += timeDelta;
    //            m_leftOverTicks = 0;
    //            m_frameCount++;

    //            update();
    //        }

    //        // Track the current framerate.
    //        if (m_frameCount != lastFrameCount)
    //            m_framesThisSecond++;

    //        if (m_qpcSecondCounter >= static_cast<u64>(m_qpcFrequency.QuadPart))
    //        {
    //            m_framesPerSecond = m_framesThisSecond;
    //            m_framesThisSecond = 0;
    //            m_qpcSecondCounter %= static_cast<u64>(m_qpcFrequency.QuadPart);
    //        }
    //    }

    // private:
    //     // Source timing data uses QPC units.
    //     u64 m_qpcFrequency;
    //     u64 m_qpcLastTime;
    //     u64 m_qpcMaxDelta;

    //    // Derived timing data uses a canonical tick format.
    //    u64 m_elapsedTicks;
    //    u64 m_totalTicks;
    //    u64 m_leftOverTicks;

    //    // Members for tracking the framerate.
    //    u64 m_frameCount;
    //    u64 m_framesPerSecond;
    //    u64 m_framesThisSecond;
    //    u64 m_qpcSecondCounter;

    //    // Members for configuring fixed timestep mode.
    //    bool m_isFixedTimeStep;
    //    u64 m_targetElapsedTicks;
    //};
}
