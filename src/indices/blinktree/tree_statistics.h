#pragma once

#include "node.h"
#include "tree.h"
#include <config.h>
#include <cstdint>
#include <ostream>

namespace indices::blinktree {
class TreeStatistics
{
public:
    TreeStatistics(const Tree *tree) : _tree_height(tree->height()) {}

    void collect_node_statistics(Node *node);

    inline std::size_t count_inner_nodes() const { return _count_inner_nodes; }
    inline std::size_t count_leaf_nodes() const { return _count_leaf_nodes; }
    inline std::size_t count_inner_node_keys() const { return _count_inner_node_keys; }
    inline std::size_t count_leaf_node_keys() const { return _count_leaf_node_keys; }
    inline std::size_t tree_height() const { return _tree_height; }

    friend std::ostream &operator<<(std::ostream &stream, const TreeStatistics &tree_statistics)
    {
        const auto count_nodes = tree_statistics.count_leaf_nodes() + tree_statistics.count_inner_nodes();
        const auto size_in_bytes = count_nodes * Config::node_size();
        stream << "Statistics of the Tree: \n"
               << "  Node size:     " << sizeof(Node) << " B\n"
               << "  Header size:   " << sizeof(NodeHeader) << " B\n"
               << "  Inner keys:    " << InnerNode::max_keys << " (" << sizeof(key_type) * InnerNode::max_keys
               << " B)\n"
               << "  Leaf keys:     " << LeafNode::max_items << " (" << sizeof(key_type) * LeafNode::max_items
               << " B)\n"
               << "  Tree height:   " << tree_statistics.tree_height() << "\n"
               << "  Inner nodes:   " << tree_statistics.count_inner_nodes() << "\n"
               << "  Inner entries: " << tree_statistics.count_inner_node_keys() << "\n"
               << "  Leaf nodes:    " << tree_statistics.count_leaf_nodes() << "\n"
               << "  Leaf entries:  " << tree_statistics.count_leaf_node_keys() << "\n"
               << "  Nodes size:    " << size_in_bytes / 1024.0 / 1024.0 << " MB";

        return stream;
    }

private:
    std::size_t _count_inner_nodes = 0;
    std::size_t _count_leaf_nodes = 0;
    std::size_t _count_inner_node_keys = 0;
    std::size_t _count_leaf_node_keys = 0;
    std::size_t _tree_height;
};
} // namespace indices::blinktree