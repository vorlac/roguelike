#pragma once

#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <chrono>
#include <concepts>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <mutex>
#include <set>
#include <thread>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "core/assert.hpp"
#include "utils/concepts.hpp"

namespace rl::ds {
    using namespace std::chrono_literals;

    enum class BufferItemStatus : uint_fast8_t {
        None = 0,
        Valid,
        Timeout,
        Partial,
        Unknown
    };

    template <std::movable TElem, auto BufferSize = 512U>
    class ring_buffer
    {
    public:
        using data_elem_t = TElem;
        using buff_size_t = decltype(BufferSize);
        using container_t = std::array<TElem, BufferSize>;

    public:
        u32 push(TElem item)
        {
            // if the buffer happens to be full, block any threads
            // trying to transfer a new item to the buffer until
            // notified that the buffer has at least one vacancy
            std::unique_lock<std::mutex> lock(m_buffer_lock);
            m_buffer_not_full_cv.wait(lock, [&]() {
                return m_vacancies > 0;
            });

            runtime_assert(m_occupancies <= this->m_buffer.size(),
                           "ringbuffer reporting more occupied slots than the max buffer size:\n"
                               << "  occupancies = " << m_occupancies << std::endl
                               << "  vacancies   = " << m_vacancies << std::endl
                               << "  buffer_size = " << m_buffer.size() << std::endl);

            // transfer the item to the buffer at the current
            // tail of the then increment the write index
            m_buffer[m_write_idx] = std::move(item);
            ++m_write_idx %= m_buffer.size();

            // update counts to reflect the buffer
            // gaining an item and losing a vacant slot
            ++m_occupancies;
            --m_vacancies;

            // maintain a count of all elements that pass through the buffer
            m_total_passthrough.fetch_add(1, std::memory_order_relaxed);

            // notify any threads waiting to pop an item
            // off of the buffer that it's no longer empty
            m_buffer_not_empty_cv.notify_one();

            // return a count of items occupying
            // the buffer after this item was added
            return m_occupancies;
        }

        std::pair<BufferItemStatus, TElem> pop()
        {
            // if the buffer happens to be empty, block any threads
            // from moving any further until an item is added to
            // the buffer or the 2.5s timeout is reached
            std::unique_lock<std::mutex> lock(m_buffer_lock);
            if (m_buffer_not_empty_cv.wait_for(lock, 2.5s, [&]() {
                    return m_occupancies > 0;
                }))
            {
                // remove the item from the buffer, then update
                // the the read index to the next available slot
                auto item = std::move(m_buffer[m_read_idx]);
                ++m_read_idx %= m_buffer.size();

                // update counts to reflect that we lost an
                // item from the buffer and gained an empty slot
                ++m_vacancies;
                --m_occupancies;

                // notify any threads that might be waiting to
                // push an item onto the buffer at the other cv
                m_buffer_not_full_cv.notify_one();

                // transfer ownership of item to caller
                return { BufferItemStatus::Valid, std::forward<TElem>(item) };
            }
            else
            {
                // if we land in here then this thread timed out waiting for
                // the condition to be met above. this might mean nothing has
                // been added to the buffer yet/recently and it's empty, but
                // could also mean the threads writing to the buffer are done
                // with their work. The calling code is responsible for handling
                return { BufferItemStatus::Timeout, TElem() };
            }
        }

    private:
        container_t m_buffer = {};

        buff_size_t m_read_idx{ 0 };
        buff_size_t m_write_idx{ 0 };
        buff_size_t m_vacancies{ m_buffer.size() };
        std::atomic<size_t> m_total_passthrough{ 0 };
        std::atomic<buff_size_t> m_occupancies{ 0 };

        std::mutex m_buffer_lock{ std::mutex{} };
        std::mutex m_taskinfo_lock{ std::mutex{} };
        std::condition_variable m_buffer_not_full_cv{};
        std::condition_variable m_buffer_not_empty_cv{};
    };

    template <typename T>
    using buffer_ptr_t = std::shared_ptr<ring_buffer<T>>;
}
