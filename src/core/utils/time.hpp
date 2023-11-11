#pragma once

#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>
#include <vector>

#include <fmt/chrono.h>
#include <fmt/format.h>

#include "core/utils/io.hpp"

#ifdef WIN32
  #ifndef LEAN_AND_MEAN
    #define LEAN_AND_MEAN
    #include <Windows.h>
  #endif
#endif

namespace rl::perf
{
    // https://blog.bearcats.nl/perfect-sleep-function/
    // struct sleep
    // {
    //     #include <chrono>
    //     #include <thread>
    //
    //    #include <math.h>
    //    using namespace std;
    //    using namespace chrono;
    //
    //    void preciseSleep(double seconds)
    //    {
    //        static double estimate = 5e-3;
    //        static double mean = 5e-3;
    //        static double m2 = 0;
    //        static int64_t count = 1;
    //        while (seconds > estimate)
    //        {
    //            auto start = high_resolution_clock::now();
    //            this_thread::sleep_for(milliseconds(1));
    //            auto end = high_resolution_clock::now();
    //            double observed = (end - start).count() / 1e9;
    //            seconds -= observed;
    //            ++count;
    //            double delta = observed - mean;
    //            mean += delta / count;
    //            m2 += delta * (observed - mean);
    //            double stddev = sqrt(m2 / (count - 1));
    //            estimate = mean + stddev;
    //        }
    //        auto start = high_resolution_clock::now();
    //        auto spinNs = (int64_t)(seconds * 1e9);
    //        auto delay = nanoseconds(spinNs) while (high_resolution_clock::now() - start < delay);
    //    }
    // };

    // struct timer
    // {
    // private:
    //     uint64_t m_tick_frequency{ 0 };
    //     uint64_t m_start_hpc_tick{ 0 };
    //     uint64_t m_start_microsec{ 0 };
    //     uint64_t m_last_timestamp{ 0 };

    // public:
    //     timer()
    //     {
    //         LARGE_INTEGER tick_frequency{ 0 };
    //         BOOL ret = QueryPerformanceFrequency(&tick_frequency);
    //         if (ret == 0)
    //             fmt::print("QueryPerformanceFrequency failed\n");

    //         LARGE_INTEGER start_hpc_tick{ 0 };
    //         ret = QueryPerformanceCounter(&start_hpc_tick);
    //         if (ret == 0)
    //             fmt::print("QueryPerformanceCounter failed\n");

    //         LARGE_INTEGER current_tick{ 0 };
    //         ret = QueryPerformanceCounter(&current_tick);
    //         if (ret == 0)
    //             fmt::print("QueryPerformanceCounter failed\n");

    //         m_tick_frequency = tick_frequency.QuadPart;
    //         m_start_hpc_tick = start_hpc_tick.QuadPart;
    //         m_start_microsec = ((current_tick.QuadPart - m_start_hpc_tick) * 1000000) /
    //                            m_tick_frequency;
    //         m_last_timestamp = m_start_microsec;
    //     }

    //     [[nodiscard]]
    //     inline uint64_t now()
    //     {
    //         LARGE_INTEGER current_tick{ 0 };
    //         QueryPerformanceCounter(&current_tick);
    //         m_last_timestamp = current_tick.QuadPart;
    //         return m_last_timestamp;
    //     }

    //     /**
    //      * @brief get elapsed microseconds
    //      * */
    //     [[nodiscard]]
    //     inline uint64_t elapsed_mu()
    //     {
    //         uint64_t curr_timestamp = timer::now();
    //         m_last_timestamp = ((curr_timestamp - m_start_hpc_tick) * 1000000) /
    //         m_tick_frequency; return m_last_timestamp;
    //     }

    //     /**
    //      * @brief get elapsed milliseconds
    //      * */
    //     [[nodiscard]]
    //     inline double elapsed_ms()
    //     {
    //         uint64_t microseconds = elapsed_mu();
    //         return static_cast<double>(microseconds) / 1000.0;
    //     }

    //     /**
    //      * @brief get elapsed seconds
    //      * */
    //     [[nodiscard]]
    //     inline double elapsed_sec()
    //     {
    //         double milliseconds = elapsed_ms();
    //         return milliseconds / 1000.0;
    //     }
    // };
}

namespace rl
{
    using namespace std::chrono_literals;

    template <class... Durations, class DurationIn>
    std::tuple<Durations...> to_durations(DurationIn d)
    {
        std::tuple<Durations...> retval;
        using discard = std::array;
        discard{ 0,
                 (void((std::get<Durations>(retval) = std::chrono::duration_cast<Durations>(d)),
                       (d -= std::chrono::duration_cast<DurationIn>(std::get<Durations>(retval)))),
                  0)... };
        return retval;
    }

    template <typename TRep = double, typename TPeriod = std::milli>
    struct timer
    {
        using rep_t = TRep;
        using preiod_t = TPeriod;
        using duration_t = std::chrono::duration<TRep, TPeriod>;
        using clock_t = std::chrono::high_resolution_clock;

        timer(std::string label)
            : m_label{ std::forward<std::string>(label) }
        {
        }

        template <typename... TArgs>
        [[maybe_unused]] inline decltype(auto) measure(auto function, TArgs... args)
        {
            m_prev_delta_time = clock_t::now();

            auto ret = function(args...);

            this->print_delta_time();
            return ret;
        }

        inline void delta_update()
        {
            m_prev_delta_time = clock_t::now();
        }

        template <typename TOut = duration_t>
        inline TOut delta_time()
        {
            const clock_t::time_point now{ clock_t::now() };
            const duration_t delta{ now - m_prev_delta_time };
            m_prev_delta_time = std::move(now);
            return timer::convert<TOut>(delta);
        }

        template <typename TOut = duration_t>
        [[nodiscard]] inline TOut elapsed()
        {
            const clock_t::time_point now{ clock_t::now() };
            const duration_t elapsed{ now - m_start_time };
            return timer::convert<TOut>(elapsed);
        }

        inline void print_delta_time()
        {
            auto dt{ this->delta_time() };
            log::info("     > {} => [{}]", m_label, dt);
        }

        template <typename TOut>
        inline static constexpr auto convert(const duration_t& duration)
        {
            if constexpr (std::is_same_v<TOut, duration_t>)
                return duration;
            else if constexpr (std::is_same_v<TOut, rep_t>)
                return duration.count();
            else if constexpr (std::is_same_v<TOut, std::string> && std::is_floating_point_v<rep_t>)
                return TOut(fmt::format("{:.2L}", duration.count()));
            else if constexpr (std::is_same_v<TOut, std::string> && !std::is_floating_point_v<rep_t>)
                return TOut(fmt::format("{:L}", duration.count()));
        }

        const std::string m_label{};
        const clock_t::time_point m_start_time{ clock_t::now() };
        mutable clock_t::time_point m_prev_delta_time{ m_start_time };
    };
}
