#pragma once

#include <array>
#include <string>

#include "utils/numeric.hpp"

namespace rl::utf8 {
    using codepoint = u32;

    constexpr u8 ONE_BYTE_MASK{ 0x80 };                 // 1000 0000
    constexpr u8 TWO_BYTE_MASK{ 0xE0 };                 // 1110 0000
    constexpr u8 THREE_BYTE_MASK{ 0xF0 };               // 1111 0000
    constexpr u8 FOUR_BYTE_MASK{ 0xF8 };                // 1111 1000
    constexpr u8 SURROGATE_MASK{ 0xC0 };                // 1100 0000
                                                        //
    constexpr u8 ONE_BYTE_MARKER{ 0x00 };               // 0000 0000
    constexpr u8 TWO_BYTE_MARKER{ 0xC0 };               // 1100 0000
    constexpr u8 THREE_BYTE_MARKER{ 0xE0 };             // 1110 0000
    constexpr u8 FOUR_BYTE_MARKER{ 0xF0 };              // 1111 0000
    constexpr u8 SURROGATE_MARKER{ 0x80 };              // 1000 0000
                                                        //
    constexpr u8 INVALID_BYTE_MARKER_AND_MASK{ 0xF8 };  // 1111'1000

    struct unicode_block final
    {
        size_t length{ 0 };
        std::array<u8, 5> buffer{};
    };

    constexpr u32 codepoint_byte_size(const u8 cp)
    {
        if ((ONE_BYTE_MASK & cp) == ONE_BYTE_MARKER)
            return 1;
        if ((TWO_BYTE_MASK & cp) == TWO_BYTE_MARKER)
            return 2;
        if ((THREE_BYTE_MASK & cp) == THREE_BYTE_MARKER)
            return 3;
        if ((FOUR_BYTE_MASK & cp) == FOUR_BYTE_MARKER)
            return 4;
        return 0;
    }

    constexpr u64 codepoint_size_as_utf8(const codepoint cp)
    {
        return cp < 0x80        ? 1
             : cp < 0x800       ? 2
             : cp < 0x10000     ? 3
             : cp < 0x200000    ? 4
             : cp < 0x4000000   ? 5
             : cp <= 0x7fffffff ? 6
                                : 0;
    }

    // Given a pointer to a valid utf8 character, produce the codepoint
    constexpr codepoint utf8_to_codepoint(const char* const str)
    {
        // debug_assert(p != nullptr);
        switch (codepoint_byte_size(static_cast<u8>(*str))) {
            case 1:
                return static_cast<codepoint>(
                    str[0]);

            case 2:
                return static_cast<codepoint>(
                    (str[0] & 0x1F) << 6 |
                    (str[1] & 0x3F));

            case 3:
                return static_cast<codepoint>(
                    (str[0] & 0x0F) << 12 |
                    (str[1] & 0x3F) << 6 |
                    (str[2] & 0x3F));

            case 4:
                return static_cast<codepoint>(
                    (str[0] & 0x07) << 18 |
                    (str[1] & 0x3F) << 12 |
                    (str[2] & 0x3F) << 6 |
                    (str[3] & 0x3F));

            case 0:
                [[fallthrough]];
            default:
                // debug_assert("invalid utf-8 string: {}", str);
                return 0;
        }
    }

    constexpr unicode_block codepoint_to_utf8(const codepoint cp)
    {
        switch (codepoint_size_as_utf8(cp)) {
            default:
            case 0:
                return unicode_block{
                    .length = 0,
                    .buffer = { 0, 0, 0, 0, 0 }
                };
            case 1:
                return unicode_block{
                    .length = 1,
                    .buffer = {
                        static_cast<u8>(cp),
                        0,
                        0,
                        0,
                        0,
                    }
                };
            case 2:
                return unicode_block{
                    .length = 2,
                    .buffer = {
                        static_cast<u8>(TWO_BYTE_MARKER | ((cp >> 6) & 0x1F)),
                        static_cast<u8>(SURROGATE_MARKER | (cp & 0x3F)),
                        0,
                        0,
                        0,
                    }
                };
            case 3:
                return unicode_block{
                    .length = 3,
                    .buffer = {
                        static_cast<u8>(THREE_BYTE_MARKER | (cp >> 12) & 0x0F),
                        static_cast<u8>(SURROGATE_MARKER | ((cp >> 6) & 0x3F)),
                        static_cast<u8>(SURROGATE_MARKER | (cp & 0x3F)),
                        0,
                        0,
                    }
                };
            case 4:
                return unicode_block{
                    .length = 4,
                    .buffer = {
                        static_cast<u8>((FOUR_BYTE_MARKER | ((cp >> 18) & 0x07))),
                        static_cast<u8>(SURROGATE_MARKER | ((cp >> 12) & 0x3F)),
                        static_cast<u8>(SURROGATE_MARKER | ((cp >> 6) & 0x3F)),
                        static_cast<u8>(SURROGATE_MARKER | (cp & 0x3F)),
                        0,
                    }
                };
        }
    }

    constexpr std::string codepoint_to_str(codepoint cp)
    {
        std::array<char, 8> ret{};
        switch (codepoint_size_as_utf8(cp)) {
            case 6:
                ret[5] = static_cast<char>(SURROGATE_MARKER | (cp & 0x3F));
                cp >>= 6;
                cp |= 0x4000000;
                [[fallthrough]];
            case 5:
                ret[4] = static_cast<char>(SURROGATE_MARKER | (cp & 0x3F));
                cp >>= 6;
                cp |= 0x200000;
                [[fallthrough]];
            case 4:
                ret[3] = static_cast<char>(SURROGATE_MARKER | (cp & 0x3F));
                cp >>= 6;
                cp |= 0x10000;
                [[fallthrough]];
            case 3:
                ret[2] = static_cast<char>(SURROGATE_MARKER | (cp & 0x3F));
                cp >>= 6;
                cp |= 0x800;
                [[fallthrough]];
            case 2:
                ret[1] = static_cast<char>(SURROGATE_MARKER | (cp & 0x3F));
                cp >>= 6;
                cp |= 0xC0;
                [[fallthrough]];
            case 1:
                ret[0] = static_cast<char>(cp);
                break;

            case 0:
                [[fallthrough]];
            default:
                break;
        }

        return ret.data();
    }

    constexpr std::string codepoint_to_str_old(codepoint c)
    {
        char seq[8]{};

        i32 n{ 0 };
        if (c < 0x80)
            n = 1;
        else if (c < 0x800)
            n = 2;
        else if (c < 0x10000)
            n = 3;
        else if (c < 0x200000)
            n = 4;
        else if (c < 0x4000000)
            n = 5;
        else if (c <= 0x7fffffff)
            n = 6;

        seq[n] = '\0';

        switch (n) {
            case 6:
                seq[5] = 0x80 | (c & 0x3f);
                c = c >> 6;
                c |= 0x4000000;
                [[fallthrough]];
            case 5:
                seq[4] = 0x80 | (c & 0x3f);
                c = c >> 6;
                c |= 0x200000;
                [[fallthrough]];
            case 4:
                seq[3] = 0x80 | (c & 0x3f);
                c = c >> 6;
                c |= 0x10000;
                [[fallthrough]];
            case 3:
                seq[2] = 0x80 | (c & 0x3f);
                c = c >> 6;
                c |= 0x800;
                [[fallthrough]];
            case 2:
                seq[1] = 0x80 | (c & 0x3f);
                c = c >> 6;
                c |= 0xc0;
                [[fallthrough]];
            case 1:
                seq[0] = static_cast<char>(c);
        }
        return { seq, seq + n };
    }
}
