#pragma once

#include "utils/assert.hpp"
#include "utils/concepts.hpp"

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

namespace rl
{
    using namespace std::chrono_literals;

    enum class BufferItemStatus : uint_fast8_t
    {
        None = 0,
        Valid,
        Timeout,
        Partial,
        Unknown
    };

    struct BufferSubscriber
    {
        enum class State
        {
            Invalid,
            Initializing,
            Active,
            Teardown,
            Finished,
        };

        State state{ State::Invalid };
    };

    template <std::movable TElem, auto BufferSize = 512U>
        requires PositiveInteger<BufferSize>
    class ring_buffer
    {
    public:
        using data_elem_t = TElem;
        using buff_size_t = decltype(BufferSize);
        using container_t = std::array<TElem, BufferSize>;

    public:
        uint32_t push(TElem item)
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

        auto pop() -> std::pair<BufferItemStatus, TElem>
        {
            // if the buffer happens to be empty, block any threads
            // from moving any further until an item is added to
            // the buffer or the 2.5s timeout is reached
            std::unique_lock<std::mutex> lock(m_buffer_lock);
            if (m_buffer_not_empty_cv.wait_for(lock, 2.5s, [&]() {
                    return m_occupancies > 0;
                }))
            {
                // remove the item fromm the buffer, then update
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
                // the condition to be met above. this might mean nohing has
                // been added to the buffer yet/recently and it's empty, but
                // could also mean the threads writing to the buffer are done
                // with their work. The calling code is responsible for handling
                return { BufferItemStatus::Timeout, TElem() };
            }
        }

        inline buff_size_t active_readers()
        {
            buff_size_t ret = 0;

            std::scoped_lock<std::mutex> lock(m_taskinfo_lock);
            for (auto&& reader : m_registered_readers)
            {
                auto state = reader->get_state();
                if (state == task::State::Initializing || state == task::State::Active)
                    ++ret;
            }

            return ret;
        }

        inline buff_size_t active_writers()
        {
            buff_size_t ret = 0;

            std::scoped_lock<std::mutex> lock(m_taskinfo_lock);
            for (auto&& writer : m_registered_writers)
            {
                auto state = writer->get_state();
                if (state == task::State::Initializing || state == task::State::Active)
                    ++ret;
            }

            return ret;
        }

        inline bool is_active()
        {
            // the buffer is active if it's holding any elements
            if (m_occupancies.load(std::memory_order_relaxed) > 0)
                return true;

            // if the buffer still hasn't received a single element
            // yet, then it most likely still initializing itself.
            if (m_total_passthrough == 0)
                return true;

            // if the buffer is empty, there's a chance it's just
            // emptying faster than it can be populated. now the
            // state of all registered write tasks should be checked
            // to see if there's a chance any more data will be added
            //
            // TODO: add a count for this so we don't
            //       have to grab a lock to check it
            auto writer_count = active_writers();
            if (writer_count > 0)
                return true;

            // if the buffer isn't holding any data and there
            // aren't any actively running tasks working with
            // the buffer then it's no longer in active use.
            return false;
        }

        inline void register_reader(std::string reader_name)
        {
            std::scoped_lock<std::mutex> lock(m_taskinfo_lock);
            m_registered_readers.push_back(status);
        }

        inline void register_writer(std::string status)
        {
            std::scoped_lock<std::mutex> lock(m_taskinfo_lock);
            m_registered_writers.push_back(status);
        }

        inline float utilization()
        {
            runtime_assert(
                m_vacancies + m_occupancies.load(std::memory_order_relaxed) == m_buffer.size(),
                "Buffer counts are off by "
                    << m_buffer.size() - (m_occupancies.load(std::memory_order_relaxed) + m_vacancies)
                    << std::endl
                    << "  size = " << m_buffer.size() << std::endl
                    << "  items = " << m_occupancies << std::endl
                    << "  openings = " << m_vacancies << std::endl);

            return 100.0
                   * (m_occupancies.load(std::memory_order_relaxed)
                      / static_cast<float>(m_buffer.size()));
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

        std::vector<std::string> m_registered_writers = {};
        std::vector<std::string> m_registered_readers = {};
    };

    template <typename T>
    using buffer_ptr_t = std::shared_ptr<ring_buffer<T>>;
}