// inline static void handle_inputs(timer& time)
// {
//     constexpr auto sleep_time_ms = 3.22;
//     const auto start = time.elapsed_ms();
//     while (time.elapsed_ms() - start <= sleep_time_ms)
//         ;  // spin to sleep for 2 ms
// };

// inline static void physics_update(timer& time)
// {
//     constexpr auto sleep_time_ms = 3.12;
//     const auto start = time.elapsed_ms();
//     while (time.elapsed_ms() - start <= sleep_time_ms)
//         ;  // spin to sleep for 4 ms
// };

// inline static void render(timer& time)
// {
//     constexpr auto sleep_time_ms = 5.58;
//     const auto start = time.elapsed_ms();
//     while (time.elapsed_ms() - start <= sleep_time_ms)
//         ;  // spin to sleep for 8 ms
// };

// inline static void print_stats(timer& time, auto delta_time, auto time_since_render,
//                                auto input_count, auto update_count, auto render_count)
// {
//     auto&& secs{ time.elapsed_sec() };
//     auto&& milli{ time.elapsed_ms() };
//     auto&& avg_dt = milli / input_count;
//     auto&& avg_input_per_s = input_count / secs;
//     auto&& avg_update_per_s = update_count / secs;
//     auto&& avg_render_per_s = render_count / secs;

//     fmt::print(
//         "[i={:>4} u={:>4} r={:>4}] dt={:>6.2f}ms, avg_dt={:>6.2f}ms, last_ren={:>6.2f}ms,
//         ips={:6.2f} ups={:6.2f} rps={:6.2f}\n", input_count, update_count, render_count,
//         delta_time, avg_dt, time_since_render, avg_input_per_s, avg_update_per_s,
//         avg_render_per_s);
// }

// int main(int argc, char** argv)
// {
//     timer time{};
//     std::ios_base::sync_with_stdio(false);

//     auto delta_time{ 0.0 };
//     auto update_delay{ 0.0 };
//     auto time_since_render{ 0.0 };
//     auto curr_time{ 0.0 };
//     auto prev_time{ time.elapsed_ms() };
//     auto start_time{ 0.0 };
//     auto prev_render{ 0.0 };

//     static constexpr auto MS_PER_UPDATE{ 1000.0 / 60.0 };
//     static constexpr auto MS_PER_RENDER{ 1000.0 / 120.0 };

//     uint32_t input_count = 0;
//     uint32_t update_count = 0;
//     uint32_t render_count = 0;

//     while (true)
//     {
//         curr_time = time.elapsed_ms();
//         delta_time = curr_time - prev_time;
//         prev_time = curr_time;
//         update_delay += delta_time;

//         handle_inputs(time);

//         if (++input_count % 60 == 0)
//             print_stats(time, delta_time, time_since_render, input_count, update_count,
//                         render_count);

//         while (update_delay >= MS_PER_UPDATE)
//         {
//             physics_update(time);
//             update_delay -= MS_PER_UPDATE;
//             ++update_count;
//         }

//         time_since_render = time.elapsed_ms() - prev_render;
//         if (MS_PER_RENDER > time_since_render)
//             continue;

//         prev_render = time.elapsed_ms();
//         render(time);

//         ++render_count;
//     }

//     return 0;
// }

// static bool benchmark()
//{
//     double elapsed{ 0 };
//     u64 loop_count{ 0 };

//    srand((u32)time(nullptr));
//    auto& window{ m_sdl.window() };
//    auto renderer{ window.renderer() };
//    sdl::color color{ 0, 0, 0 };
//    sdl::application::timer_t timer{};
//    while (elapsed < 60000) [[unlikely]]
//    {
//        elapsed = timer.elapsed();

//        renderer->clear();
//        renderer->set_draw_color(color);
//        renderer->present();

//        if (loop_count++ % 960 == 0)
//        {
//            double dt = timer.elapsed() - elapsed;
//            // const double avg_dlt{ ((elapsed_ms * 1000.0) / cast::to<f64>(loop_count)) };
//            // const double avg_ups{ (cast::to<f64>(loop_count) / elapsed_ms) };
//            fmt::print("[prev_dt={:<6.4f}ms] [avg_dlt={:<6.4f}ms] [avg_ups={:<6.4f}]\n", dt,
//                       ((f64)loop_count / elapsed), (elapsed * 1000.0) / (f64)loop_count);
//            color = {
//                rand() % 128,
//                rand() % 128,
//                rand() % 128,
//            };
//        }
//    }

//    return true;
//}
