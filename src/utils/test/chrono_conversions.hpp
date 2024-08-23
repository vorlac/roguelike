#include <algorithm>
#include <chrono>
#include <iomanip>
#include <print>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

// this is really the one and only function that handles all time
// duration conversions. you can convert any unit & value of time
// duration to any number of duration unit & value combinations.
template <typename... TDurations>
std::tuple<TDurations...> convert_durations(auto in_duration)
{
    std::tuple<TDurations...> ret{};
    ((std::get<TDurations>(ret) = std::chrono::duration_cast<TDurations>(in_duration),
      in_duration -= std::chrono::duration_cast<TDurations>(std::get<TDurations>(ret))),
     ...);

    return ret;
}

// this is just a convenience function that will
// create a string that represents the typename
// of any code element at compile time for gcc
template <typename T>
consteval std::string_view demangled_typename()
{
    constexpr std::string_view pref{ "with T = " };
    constexpr std::string_view suff{ "; " };
    constexpr std::string_view func{ __PRETTY_FUNCTION__ };
    constexpr auto start{ func.find(pref) + pref.size() };
    constexpr auto end{ func.find(suff) };
    constexpr auto size{ end - start };
    return func.substr(start, size);
}

void duration_conversion_example()
{
    using namespace std::chrono;
    using namespace std::literals;
    std::println("\nTime Duration Conversions:");

    // start with 123 seconds
    auto duration1{ 12345s };
    // print out the value of duration1
    std::println("  duration1 = {}", duration1);
    // convert the original time from:
    //   seconds ==> (hours, minutes, seconds)
    auto&& [h, m, s] = convert_durations<hours, minutes, seconds>(duration1);
    std::println("  {} = ({} + {} + {})", duration1, h, m, s);
    // confirm they match up to the orig value in seconds after readding them
    seconds total_seconds{ s + duration_cast<seconds>(m) + duration_cast<seconds>(h) };
    std::println("  {} == {}", duration1, total_seconds);
    // convert the original duration of seconds into hours
    auto&& [orig_to_mins] = convert_durations<minutes>(duration1);
    // print how many mins the original duration converts to
    std::println("  duration1 to minutes = {}\n", orig_to_mins);
}

void date_and_time_example()
{
    using namespace std::chrono;
    using namespace std::literals;

    const time_point<system_clock> now = system_clock::now();
    const std::time_t time_now = system_clock::to_time_t(now);
    const std::time_t time_yesterday = system_clock::to_time_t(now - 24h);
    const std::time_t time_tomorrow = system_clock::to_time_t(now + 24h);

    std::println("\nDate & Time Examples:");
    std::println("   now: {:%D %r}", now);
    std::println("   tomorrow: {:%D %r}", now + 24h);
    std::println("   yesterday: {:%D %r}", now - 24h);
}

int main()
{
    duration_conversion_example();
    date_and_time_example();
}
