#pragma once

#include <array>
#include <string_view>
#include <tuple>

#include <fmt/format.h>

namespace rl {
    template <auto& String>
    struct hash
    {
        constexpr static std::string_view str{ String };

        static consteval uint32_t rs()
        {
            constexpr uint32_t b = 378551;
            uint32_t a = 63689;
            uint32_t hash = 0;
            for (const char c : str) {
                hash = hash * a + c;
                a = a * b;
            }

            return hash;
        }

        static consteval uint32_t js()
        {
            uint32_t hash{ 1315423911 };
            for (const char c : str)
                hash ^= ((hash << 5) + c + (hash >> 2));

            return hash;
        }

        static consteval uint32_t pjw()
        {
            constexpr uint32_t bits_in_unsigned_int{ sizeof(uint32_t) * 8 };
            constexpr uint32_t three_quarters{ (bits_in_unsigned_int * 3) / 4 };
            constexpr uint32_t one_eighth{ bits_in_unsigned_int / 8 };
            constexpr uint32_t high_bits{ 0xFFFFFFFF << (bits_in_unsigned_int - one_eighth) };

            uint32_t hash{ 0 };
            uint32_t temp{ 0 };
            for (const char c : str) {
                hash = (hash << one_eighth) + c;
                if ((temp = hash & high_bits) != 0)
                    hash = ((hash ^ (temp >> three_quarters)) & (~high_bits));
            }

            return hash;
        }

        static consteval uint32_t elf()
        {
            uint32_t hash{ 0 };
            uint32_t x{ 0 };

            for (const char c : str) {
                hash = (hash << 4) + c;
                if ((x = hash & 0xF0000000L) != 0)
                    hash ^= (x >> 24);
                hash &= ~x;
            }

            return hash;
        }

        static consteval uint32_t bkdr()
        {
            constexpr uint32_t seed{ 131 };  // 31 131 1313 13131 131313 etc..
            uint32_t hash = 0;
            for (const char c : str)
                hash *= seed + c;

            return hash;
        }

        static consteval uint32_t sdbm()
        {
            uint32_t hash = 0;
            for (const char c : str)
                hash = c + (hash << 6) + (hash << 16) - hash;

            return hash;
        }

        static consteval uint32_t djb()
        {
            uint32_t hash = 5381;
            for (const char c : str)
                hash = ((hash << 5) + hash) + c;

            return hash;
        }

        static consteval uint32_t dek()
        {
            uint32_t hash = static_cast<uint32_t>(str.length());
            for (const char c : str) {
                hash = ((hash << 5) ^ (hash >> 27)) ^ c;
            }

            return hash;
        }

        static consteval uint32_t bp()
        {
            uint32_t hash = 0;
            for (const char c : str) {
                hash = hash << 7 ^ c;
            }

            return hash;
        }

        static consteval uint32_t fnv()
        {
            constexpr uint32_t fnv_prime{ 0x811C9DC5 };

            uint32_t hash = 0;
            for (const char c : str) {
                hash *= fnv_prime;
                hash ^= c;
            }

            return hash;
        }

        static consteval uint32_t ap()
        {
            uint32_t hash{ 0xAAAAAAAA };
            for (std::size_t i = 0; i < str.size(); i++) {
                hash ^= (i & 1) == 0
                          ? (hash << 7) ^ str[i] * (hash >> 3)
                          : ~((hash << 11) + (str[i] ^ (hash >> 5)));
            }

            return hash;
        }
    };
}

namespace rl::test {
    void test_hash()
    {
        constexpr static auto str{ "asdf" };
        fmt::println("ap hash   = {}", rl::hash<str>::ap());
        fmt::println("bp hash   = {}", rl::hash<str>::bp());
        fmt::println("dek hash  = {}", rl::hash<str>::dek());
        fmt::println("djb hash  = {}", rl::hash<str>::djb());
        fmt::println("elf hash  = {}", rl::hash<str>::elf());
        fmt::println("fnv hash  = {}", rl::hash<str>::fnv());
        fmt::println("js hash   = {}", rl::hash<str>::js());
        fmt::println("pjw hash  = {}", rl::hash<str>::pjw());
        fmt::println("rs hash   = {}", rl::hash<str>::rs());
        fmt::println("sdbm hash = {}", rl::hash<str>::sdbm());
    }
}
