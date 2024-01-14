#pragma once

#include <algorithm>
#include <concepts>
#include <type_traits>

#include "core/assert.hpp"
#include "ds/refcounted.hpp"

namespace rl::ds {

    template <typename T>
    class shared final
    {
    public:
        constexpr inline shared() = default;

        constexpr inline shared(T* data)
            : m_data{ data }
        {
            if (m_data != nullptr)
                m_data->acquire_ref();
        }

        constexpr inline shared(const ds::shared<T>& other)
            : m_data{ other.m_data }
        {
            // acquire a reference to existing
            if (m_data != nullptr)
                m_data->acquire_ref();
        }

        constexpr inline shared(shared<T>&& other) noexcept
            : m_data{ other.m_data }
        {
            other.m_data = nullptr;
        }

        constexpr virtual ~shared()
        {
            if (m_data != nullptr)
                m_data->release_ref();
        }

        constexpr inline shared<T>& operator=(shared<T>&& other) noexcept
        {
            // release reference on existing data
            // if one was already being held to it
            if (m_data != nullptr)
            {
                m_data->release_ref();
                m_data = nullptr;
            }

            std::swap(other.m_data, m_data);
            return *this;
        }

        constexpr inline shared<T>& operator=(const shared<T>& other)
        {
            if (other.m_data != nullptr)
                other.m_data->acquire_ref();
            if (m_data != nullptr)
                m_data->release_ref();

            m_data = other.m_data;
            return *this;
        }

        constexpr inline shared<T>& operator=(T* data)
        {
            if (data != nullptr)
                data->acquire_ref();
            if (m_data != nullptr)
                m_data->release_ref();

            m_data = data;
            return *this;
        }

        constexpr inline bool operator==(const shared<T>& other) const
        {
            // address comparison to guarantee it's
            // the same exact shared<T> object ref
            return m_data == other.m_data;
        }

        constexpr inline bool operator==(const T* data) const
        {
            // address comparison to guarantee it's
            // the same exact data being referenced
            return m_data == data;
        }

        constexpr inline bool operator!=(const shared<T>& other) const
        {
            return !this->operator==(other);
        }

        constexpr inline bool operator!=(const T* data) const
        {
            return !this->operator==(data);
        }

        constexpr inline T* operator->()
        {
            return m_data;
        };

        constexpr const inline T* operator->() const
        {
            return m_data;
        }

        constexpr inline T& operator*()
        {
            runtime_assert(m_data != nullptr, "dereferencing null shared<T>");
            return *m_data;
        }

        constexpr const inline T& operator*() const
        {
            runtime_assert(m_data != nullptr, "dereferencing null shared<T>");
            return *m_data;
        }

        constexpr inline operator T*()
        {
            return m_data;
        }

        constexpr const inline T* get() const
        {
            return m_data;
        }

    private:
        T* m_data{ nullptr };
    };
}
