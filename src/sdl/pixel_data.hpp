#pragma once

#include <vector>

#include "core/numeric_types.hpp"
#include "ds/dims.hpp"
#include "ds/point.hpp"
#include "ds/rect.hpp"
#include "sdl/defs.hpp"
#include "utils/assert.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL_pixels.h>
SDL_C_LIB_END

namespace rl::sdl {

    class PixelData
    {
    public:
        struct format
        {
            using type = SDL3::SDL_PixelFormatEnum;
            constexpr static inline auto Unknown = SDL3::SDL_PIXELFORMAT_UNKNOWN;
            constexpr static inline auto RGB24 = SDL3::SDL_PIXELFORMAT_RGB24;
        };

        struct structure
        {
            using type = SDL3::SDL_PixelType;
            constexpr static inline type Unknown = SDL3::SDL_PIXELTYPE_UNKNOWN;
            constexpr static inline type Index1 = SDL3::SDL_PIXELTYPE_INDEX1;
            constexpr static inline type Index4 = SDL3::SDL_PIXELTYPE_INDEX4;
            constexpr static inline type Index8 = SDL3::SDL_PIXELTYPE_INDEX8;
            constexpr static inline type Packed8 = SDL3::SDL_PIXELTYPE_PACKED8;
            constexpr static inline type Packed16 = SDL3::SDL_PIXELTYPE_PACKED16;
            constexpr static inline type Packed32 = SDL3::SDL_PIXELTYPE_PACKED32;
            constexpr static inline type Array8 = SDL3::SDL_PIXELTYPE_ARRAYU8;
            constexpr static inline type Array16 = SDL3::SDL_PIXELTYPE_ARRAYU16;
            constexpr static inline type Array32 = SDL3::SDL_PIXELTYPE_ARRAYU32;
            constexpr static inline type ArrayF16 = SDL3::SDL_PIXELTYPE_ARRAYF16;
            constexpr static inline type ArrayF32 = SDL3::SDL_PIXELTYPE_ARRAYF32;
        };

    public:
        constexpr PixelData()
        {
        }

        constexpr PixelData(const ds::dims<i32>& dims,
                            PixelData::format::type pixel_format = PixelData::format::RGB24)
            : m_format{ pixel_format }
            , m_structure{ structure::Packed32 }
        {
            u8 pixel_byte_size = 0;
            switch (m_format)
            {
                case PixelData::format::RGB24:
                    m_structure = structure::Packed32;
                    pixel_byte_size = 3;  // R,G,B - each 1 byte
                    break;
                default:
                    break;
            }

            runtime_assert(pixel_byte_size > 0, "undetermined pixel size");
            m_buffer_size = dims.area() * pixel_byte_size;
            m_data.reserve(m_buffer_size);
        }

        // {  0,  1,  2,  3,  4 },
        // {  5,  6,  7,  8,  9 },
        // { 10, 11, 12, 13, 14 },
        // { 15, 16, 17, 18, 19 },
        // { 20, 21, 22, 23, 24 },

        u8* get_col_data(const i32 x)
        {
            runtime_assert(x > m_bounds.width(), "column lookup out of bounds");
            const i32 buffer_offset{ x };
            u8* col_start_ptr{ m_data.data() + buffer_offset };
            return col_start_ptr;
        }

        u8* get_row_data(const i32 y)
        {
            const i32 buffer_offset{ m_bounds.width() * y };
            u8* row_start_ptr{ m_data.data() + buffer_offset };
            return row_start_ptr;
        }

        u8* get_pixel(const i32 x, const i32 y)
        {
            u8* row_data{ this->get_row_data(y) };
            u8* col_data{ row_data + x };
            return col_data;
        }

        // std::vector<u8> copy_pixels()
        //{
        // }

    private:
        u32 m_buffer_size{ 0 };
        ds::rect<i32> m_bounds{ 0, 0, 0, 0 };
        PixelData::format::type m_format{ PixelData::format::Unknown };
        PixelData::structure::type m_structure{ structure::Unknown };
        std::vector<u8> m_data{};
    };
};
