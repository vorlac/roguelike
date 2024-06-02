#pragma once

#include <memory>
#include <optional>
#include <vector>

#include "ds/point.hpp"
#include "utils/concepts.hpp"

namespace rl::ds {

    class Graph
    {
    public:
        template <rl::numeric T>
        class Node
        {
            i32 id{ -1 };
            std::vector<std::shared_ptr<Link>> adjacent_links{};
        };

        template <rl::numeric T>
        class Link
        {
            i32 id{ -1 };
            std::shared_ptr<Node> start_node{};
            std::shared_ptr<Node> end_node{};
        };

    private:
        std::vector<std::shared_ptr<Node<f32>>> m_nodes{};
        std::vector<std::shared_ptr<Link<f32>>> m_links{};
    };
}
