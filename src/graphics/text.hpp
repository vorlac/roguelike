#pragma once

#include <parallel_hashmap/phmap.h>

#include "graphics/vg/nanovg.hpp"
#include "resources/fonts.hpp"
#include "utils/numeric.hpp"

namespace rl {
    namespace font {
        using handle = i32;
        using Map = phmap::flat_hash_map<std::string_view, handle>;
        using Data = std::pair<std::string_view, std::basic_string_view<u8>>;

        enum class Source {
            Memory,
            Disk,
        };

        constexpr static inline i32 INVALID_FONT_HANDLE{ -1 };

        namespace style {
            constexpr static inline std::string_view Sans{ "sans" };
            constexpr static inline std::string_view SansBold{ "sans_bold" };
            constexpr static inline std::string_view Icons{ "icons" };
            constexpr static inline std::string_view Mono{ "mono" };
        }
    };

    namespace text {
        struct Properties
        {
            f32 font_size{ 18.0f };
            f32 border_thickness{ 1.0f };
            f32 border_blur{ 2.0f };

            std::string_view font{ font::style::SansBold };

            ds::color<f32> color{ Colors::White };
            ds::color<f32> border_color{ Colors::Transparent };

            ds::vector2<f32> margins{ 10.0f, 10.0f };
            nvg::Align alignment{ nvg::Align::HCenter | nvg::Align::VMiddle };
        };
    }
}