#pragma once

#include <chrono>
#include <concepts>
#include <locale>
#include <memory>
#include <string>
#include <utility>
#include <flecs.h>
#include <fmt/chrono.h>
#include <fmt/color.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <type_traits>

#include "core/ds/point.hpp"
#include "core/ds/vector2d.hpp"
#include "core/input/keymap.hpp"
#include "core/utils/assert.hpp"

namespace rl::io
{
    const static std::locale locale{ "en_US.UTF-8" };
}

namespace flecs
{
    constexpr auto format_as(const flecs::string& str)
    {
        return fmt::string_view{ str.c_str() };
    }

    constexpr auto format_as(const flecs::string_view& str)
    {
        return fmt::string_view{ str.c_str() };
    }

    auto format_as(const flecs::entity& e)
    {
        return fmt::string_view{ e.name().c_str() };
    }
}

namespace rl::ds
{
    template <typename T>
    constexpr auto format_as(const ds::vector2<T>& vec)
    {
        return fmt::format("({},{})", vec.x, vec.y);
    }
}

namespace rl::input
{
    constexpr auto format_as(GameplayAction val)
    {
        // clang-format off
        switch (val)
        {
            case GameplayAction::None: return "GameplayAction.None";
            case GameplayAction::MoveUp: return "GameplayAction.MoveUp";
            case GameplayAction::MoveDown: return "GameplayAction.MoveDown";
            case GameplayAction::MoveLeft: return "GameplayAction.MoveLeft";
            case GameplayAction::MoveRight: return "GameplayAction.MoveRight";
            case GameplayAction::RotateUp: return "GameplayAction.RotateUp";
            case GameplayAction::RotateDown: return "GameplayAction.RotateDown";
            case GameplayAction::RotateLeft: return "GameplayAction.RotateLeft";
            case GameplayAction::RotateRight: return "GameplayAction.RotateRight";
            case GameplayAction::Dash: return "GameplayAction.Dash";
            case GameplayAction::Shoot: return "GameplayAction.Shoot";
            case GameplayAction::UseItem: return "GameplayAction.UseItem";
            case GameplayAction::PrevWeapon: return "GameplayAction.PrevWeapon";
            case GameplayAction::NextWeapon: return "GameplayAction.NextWeapon";
            case GameplayAction::ToggleDebugInfo: return "GameplayAction.ToggleDebugInfo";
        }
        // clang-format on

        assert_msg(fmt::format("Gamplay Action:{} not handled in format_as", std::to_underlying(val)));
        return "GameplayAction.Unknown";
    }

    constexpr auto format_as(UIAction val)
    {
        // clang-format off
        switch (val)
        {
            case UIAction::Accept: return "UIAction.Accept";
            case UIAction::Cancel: return "UIAction.Cancel";
            case UIAction::Up: return "UIAction.Up";
            case UIAction::Down: return "UIAction.Down";
            case UIAction::Left: return "UIAction.Left";
            case UIAction::Right: return "UIAction.Right";
            case UIAction::Prev: return "UIAction.Prev";
            case UIAction::Next: return "UIAction.Next";
            case UIAction::None: return "UIAction.None";
        }
        // clang-format on

        assert_msg(fmt::format("UIAction:{} not handled in format_as", std::to_underlying(val)));
        return "UIAction.Unknown";
    }
}

namespace rl::log
{
    using namespace std::chrono_literals;

    template <auto log_level, typename... TArgs>
    inline constexpr void log(fmt::format_string<TArgs...> format_str, TArgs&&... args)
    {
        fmt::text_style style;

        if constexpr (log_level == raylib::LOG_TRACE)
            style = fmt::emphasis::faint | fmt::fg(fmt::color::red);
        else if constexpr (log_level == raylib::LOG_DEBUG)
            style = fmt::fg(fmt::color::green);
        else if constexpr (log_level == raylib::LOG_INFO)
            style = fmt::emphasis::bold | fmt::fg(fmt::color::beige);
        else if constexpr (log_level == raylib::LOG_WARNING)
            style = fmt::emphasis::bold | fmt::fg(fmt::color::yellow);
        else if constexpr (log_level >= raylib::LOG_ERROR)
            style = fmt::emphasis::bold | fmt::fg(fmt::color::red);

        fmt::print(style, fmt::format(format_str, std::forward<TArgs>(args)...) + "\n");
    }

    template <typename... TArgs>
    inline constexpr void info(fmt::format_string<TArgs...> format_str, TArgs&&... args)
    {
        log<raylib::LOG_INFO>(format_str, std::forward<TArgs>(args)...);
    }

    template <typename... TArgs>
    inline constexpr void debug(fmt::format_string<TArgs...> format_str, TArgs&&... args)
    {
        log<raylib::LOG_DEBUG>(format_str, std::forward<TArgs>(args)...);
    }

    template <typename... TArgs>
    inline constexpr void warning(fmt::format_string<TArgs...> format_str, TArgs&&... args)
    {
        log<raylib::LOG_WARNING>(format_str, std::forward<TArgs>(args)...);
    }

    template <typename... TArgs>
    inline constexpr void error(fmt::format_string<TArgs...> format_str, TArgs&&... args)
    {
        log<raylib::LOG_ERROR>(format_str, std::forward<TArgs>(args)...);
    }

    template <typename... TArgs>
    inline constexpr void fatal(fmt::format_string<TArgs...> format_str, TArgs&&... args)
    {
        log<raylib::LOG_FATAL>(format_str, std::forward<TArgs>(args)...);
        exit(-1);
    }
}
