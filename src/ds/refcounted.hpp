#pragma once

#include <atomic>

#include "utils/numeric.hpp"

namespace rl::ds {

    class refcounted
    {
    public:
        constexpr inline refcounted() = default;
        constexpr inline virtual ~refcounted() = default;

        constexpr inline refcounted(const refcounted&) noexcept
            : m_references{ 1 }
        {
        }

        constexpr inline refcounted(refcounted&&) noexcept
            : m_references{ 1 }
        {
        }

        constexpr inline refcounted& operator=(const refcounted&) noexcept
        {
            return *this;
        }

        constexpr inline refcounted& operator=(refcounted&&) noexcept
        {
            return *this;
        }

        inline void acquire_ref() const noexcept
        {
            ++m_references;
        }

        inline void release_ref() const noexcept
        {
            if (--m_references == 0)
                delete this;
        }

    private:
        mutable std::atomic<u32> m_references{ 1 };
    };
}
