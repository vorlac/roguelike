#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>

namespace rl::fs {

    inline std::filesystem::path absolute(std::filesystem::path file_path) {
        std::filesystem::path preferred{ std::move(file_path.make_preferred()) };
        return std::filesystem::absolute(preferred).make_preferred();
    }

    inline auto to_absolute(std::filesystem::path file_path) {
        return fs::absolute(std::move(file_path)).string();
    }

    template <typename TPath, typename... TArgs>
    std::filesystem::path join(const TPath& path, const TArgs&... sub_paths) {
        return { path.basic_string() + sub_paths.basic_string()... };
    }
}
