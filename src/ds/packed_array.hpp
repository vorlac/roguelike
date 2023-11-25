#pragma once

#include <memory>

#include "core/numeric_types.hpp"

namespace rl::ds {
    template <u64 Size, u32 ByteAlignment>
    class packed_array
    {
    public:
        constexpr static inline auto capacity = Size;
        constexpr static inline auto alignment = ByteAlignment;

        template <typename T, typename... Args>
        constexpr T& construct(Args&&... args)
        {
            auto t = this->data<T>();
            new (t) T(std::forward<Args>(args)...);
            return *t;
        }

        template <typename T>
        void destruct()
        {
            this->template data<T>()->~T();
        }

        template <typename T>
        T* data()
        {
            static_assert(sizeof(T) <= capacity);
            static_assert(alignof(T) <= alignment);
            return this->raw<T>();
        }

        template <typename T>
        const T* data() const
        {
            static_assert(sizeof(T) <= capacity);
            static_assert(alignof(T) <= alignment);
            return this->raw<T>();
        }

        template <typename T>
        T* raw()
        {
            return reinterpret_cast<T*>(&storage);
        }

        template <typename T>
        const T* raw() const
        {
            return reinterpret_cast<const T*>(&storage);
        }

        constexpr u64 size() const
        {
            return capacity;
        }

    private:
        alignas(ByteAlignment) std::byte storage[Size];
    };
}
