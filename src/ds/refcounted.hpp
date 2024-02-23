#pragma once

#include <atomic>
#include <memory>

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

        constexpr refcounted& operator=(const refcounted&) noexcept
        {
            return *this;
        }

        constexpr refcounted& operator=(refcounted&&) noexcept
        {
            return *this;
        }

        void acquire_ref() const noexcept
        {
            // m_references.fetch_add(1, std::memory_order_relaxed);
            ++m_references;
        }

        void release_ref() const noexcept
        {
            if (--m_references == 0)
                delete this;
        }

    private:
        // mutable std::atomic<u32> m_references{ 1 };
        mutable u32 m_references{ 1 };
    };
}
