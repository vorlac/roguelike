#pragma once

#include <atomic>

#include "utils/numeric.hpp"

namespace rl::ds {

    class refcounted
    {
    public:
        refcounted() = default;
        virtual ~refcounted() = default;

        refcounted(const refcounted&)
            : refcounted{}
        {
        }

        refcounted(refcounted&&)
            : refcounted{}
        {
        }

        refcounted& operator=(const refcounted&)
        {
            return *this;
        }

        refcounted& operator=(refcounted&&)
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
