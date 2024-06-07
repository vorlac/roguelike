#pragma once

#include <concepts>
#include <coroutine>
#include <exception>
#include <functional>
#include <iterator>
#include <type_traits>
#include <utility>

namespace rl::inline utils {
    template <typename T>
    class generator;

    namespace detail {
        template <typename T>
        class generator_promise
        {
        public:
            using val_t = std::remove_reference_t<T>;
            using ref_t = std::add_lvalue_reference_t<T>;
            using ptr_t = val_t*;

        public:
            generator_promise() = default;

            generator<T> get_return_object() noexcept
            {
                using handle = std::coroutine_handle<generator_promise<T>>;
                return generator<T>{ handle::from_promise(*this) };
            }

            constexpr std::suspend_always initial_suspend() const noexcept
            {
                return {};
            }

            constexpr std::suspend_always final_suspend() const noexcept
            {
                return {};
            }

            template <typename U = T>
                requires(!std::is_rvalue_reference_v<U>)
            std::suspend_always yield_value(std::remove_reference_t<T>& value) noexcept
            {
                m_value = std::addressof(value);
                return {};
            }

            std::suspend_always yield_value(std::remove_reference_t<T>&& value) noexcept
            {
                m_value = std::addressof(value);
                return {};
            }

            void unhandled_exception()
            {
                m_exception = std::current_exception();
            }

            void return_void()
            {
            }

            ref_t value() const noexcept
            {
                return static_cast<ref_t>(*m_value);
            }

            void rethrow_if_exception()
            {
                if (m_exception != nullptr)
                    std::rethrow_exception(m_exception);
            }

            template <typename U>
            std::suspend_never await_transform(U&& value) = delete;
            // Don't allow any use of co_await in the generator definition

        private:
            ptr_t m_value;
            std::exception_ptr m_exception;
        };

        struct generator_sentinel
        {
        };

        template <typename T>
        class generator_iterator
        {
            using handle = std::coroutine_handle<generator_promise<T>>;

        public:
            struct sentinel
            {
            };

            using iterator_category = std::input_iterator_tag;
            // What type should we use for counting elements of a potentially infinite sequence?
            using val_t = typename generator_promise<T>::val_t;
            using ref_t = typename generator_promise<T>::ref_t;
            using ptr_t = typename generator_promise<T>::ptr_t;
            using diff_t = std::ptrdiff_t;

            // Iterator needs to be default-constructible to satisfy the Range concept.
            generator_iterator() noexcept
                : m_coroutine(nullptr)
            {
            }

            explicit generator_iterator(handle coroutine) noexcept
                : m_coroutine(coroutine)
            {
            }

            friend bool operator==(const generator_iterator& it, sentinel) noexcept
            {
                return !it.m_coroutine ||
                       it.m_coroutine.done();
            }

            friend bool operator!=(const generator_iterator& it, sentinel s) noexcept
            {
                return !(it == s);
            }

            friend bool operator==(sentinel s, const generator_iterator& it) noexcept
            {
                return (it == s);
            }

            friend bool operator!=(sentinel s, const generator_iterator& it) noexcept
            {
                return it != s;
            }

            generator_iterator& operator++()
            {
                m_coroutine.resume();
                if (m_coroutine.done())
                    m_coroutine.promise().rethrow_if_exception();

                return *this;
            }

            // Need to provide post-increment operator to implement the 'Range' concept.
            void operator++(int)
            {
                (void)operator++();
            }

            ref_t operator*() const noexcept
            {
                return m_coroutine.promise().value();
            }

            ptr_t operator->() const noexcept
            {
                return std::addressof(operator*());
            }

        private:
            handle m_coroutine;
        };
    }

    template <typename T>
    class generator

    {
    public:
        using promise_type = detail::generator_promise<T>;
        using iterator_t = detail::generator_iterator<T>;

        generator(const generator<T>& other) = delete;
        generator<T>& operator=(const generator& other) = delete;

        generator() noexcept
            : m_coroutine(nullptr)
        {
        }

        generator(generator&& other) noexcept
            : m_coroutine(other.m_coroutine)
        {
            other.m_coroutine = nullptr;
        }

        ~generator()
        {
            if (m_coroutine)
                m_coroutine.destroy();
        }

        generator<T>& operator=(generator<T>&& other) noexcept
        {
            this->swap(other);
            return *this;
        }

        auto begin()
        {
            if (m_coroutine) {
                m_coroutine.resume();
                if (m_coroutine.done())
                    m_coroutine.promise().rethrow_if_exception();
            }

            return iterator_t{ m_coroutine };
        }

        auto end() noexcept
        {
            return typename iterator_t::sentinel{};
        }

        void swap(generator& other) noexcept
        {
            std::swap(m_coroutine, other.m_coroutine);
        }

    private:
        friend class detail::generator_promise<T>;

        explicit generator(std::coroutine_handle<promise_type> coroutine) noexcept
            : m_coroutine(coroutine)
        {
        }

        std::coroutine_handle<promise_type> m_coroutine;
    };

    template <typename T>
    void swap(generator<T>& a, generator<T>& b) noexcept
    {
        a.swap(b);
    }

    template <std::invocable TCallable, typename T>
    generator<std::invoke_result_t<TCallable&, typename generator<T>::iterator_t::ref_t>>
    fmap(TCallable func, generator<T> source)
    {
        for (auto&& value : source) {
            co_yield std::invoke(func, static_cast<decltype(value)>(value));
        }
    }
}
