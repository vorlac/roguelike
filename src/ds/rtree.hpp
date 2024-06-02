#pragma once

#include <memory>
#include <vector>

#include "utils/concepts.hpp"

namespace rl::ds {
    template <rl::spatial TItem, rl::numeric N = f32>
    class RTree
    {
    public:
        template <rl::spatial T>
        struct Node
        {
            std::vector<Node<T>*> children{};
        };

    public:
        bool insert(const T& item)
        {
            return true;
        }

        bool remove(const T& item)
        {
            return true;
        }

        void balance()
        {
        }

        std::vector<Node<T>*> find(const ds::rect<N>& search_rect)
        {
            return true;
        }

    private:
        std::unique_ptr<Node<T>> m_root{ nullptr };
    };
}
