#pragma once

#include <atomic>

#include "utils/numeric.hpp"

namespace rl::ds {

    class refcounted
    {
    public:
        constexpr refcounted() = default;
        virtual ~refcounted() = default;

        constexpr refcounted(const refcounted&) noexcept
        {
        }

        constexpr refcounted(refcounted&&) noexcept
        {
        }

        virtual constexpr refcounted& operator=(const refcounted&) noexcept
        {
            return *this;
        }

        virtual constexpr refcounted& operator=(refcounted&&) noexcept
        {
            return *this;
        }

        void acquire_ref() const noexcept
        {
            ++m_references;
        }

        void release_ref() const noexcept
        {
            if (--m_references == 0)
                delete this;
        }

    private:
        mutable std::atomic<u32> m_references{ 1 };
    };
}
