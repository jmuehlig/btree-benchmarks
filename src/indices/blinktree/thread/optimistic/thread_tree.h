#pragma once

#include "indices/blinktree/epoch_manager.h"
#include "indices/blinktree/node_path.h"
#include "indices/blinktree/tree.h"
#include "indices/blinktree/tree_statistics.h"
#include <util/spinlock.h>
#include <utility>

namespace indices::blinktree::thread::optimistic {
class ThreadTree : public Tree
{
public:
    ThreadTree(const std::uint16_t count_threads) : _epoch_manager(count_threads) {}
    virtual ~ThreadTree() noexcept;

    void insert(const indices::key_type key, const indices::value_type value);
    void update(const indices::key_type key, const indices::value_type value);
    value_type find(const indices::key_type key);

    void register_thread() { _epoch_manager.register_thread(); }
    void start_epoch() { _epoch_manager.start(); }
    void stop_epoch() { _epoch_manager.stop(); }

private:
    Node *locate_leaf(const key_type key, NodePath<> *node_path = nullptr);
    std::pair<Node *, indices::key_type> insert_into_leaf(Node *leaf_node, const indices::key_type key,
                                                          const indices::value_type value);
    std::pair<Node *, indices::key_type> insert_into_inner(Node *inner_node, const indices::key_type key,
                                                           Node *separator);
    void reclaim_node(Node *node)
    {
        if (node->is_inner())
        {
            for (auto i = 0u; i <= node->size(); ++i)
            {
                reclaim_node(node->separator(i));
            }
        }

        _epoch_manager.add_to_garbage(node);
    }

    EpochManager _epoch_manager;
    util::Spinlock _lock;
};
} // namespace indices::blinktree::thread::optimistic