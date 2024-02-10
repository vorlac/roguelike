#pragma once

#include <filesystem>
#include <string>
#include <string_view>

namespace rl::fs {
    inline std::filesystem::path absolute(std::filesystem::path&& file_path)
    {
        return std::filesystem::absolute(file_path.make_preferred());
    }

    template <typename TPath, typename... TArgs>
    std::filesystem::path join(TPath&& path, TArgs&&... sub_paths)
    {
        return { path.basic_string() + sub_paths.basic_string()... };
    }
}

// feb 1 - moorestown - 3:15pm
