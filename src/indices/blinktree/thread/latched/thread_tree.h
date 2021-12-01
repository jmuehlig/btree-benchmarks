#pragma once

#include "indices/blinktree/node_path.h"
#include "indices/blinktree/tree.h"
#include "indices/blinktree/tree_statistics.h"
#include <util/spinlock.h>
#include <utility>

namespace indices::blinktree::thread::latched {
class ThreadTree : public Tree
{
public:
    virtual ~ThreadTree() = default;

    void insert(const indices::key_type key, const indices::value_type value);
    void update(const indices::key_type key, const indices::value_type value);
    value_type find(const indices::key_type key);

private:
    Node *locate_leaf(const key_type key, NodePath<> *node_path = nullptr);
    std::pair<Node *, indices::key_type> insert_into_leaf(Node *leaf_node, const indices::key_type key,
                                                          const indices::value_type value);
    std::pair<Node *, indices::key_type> insert_into_inner(Node *inner_node, const indices::key_type key,
                                                           Node *separator);

    util::Spinlock _lock;

    template <bool SHARED> void lock([[maybe_unused]] Node *node)
    {
#if defined(blinktree_thread_spinlock)
        node->header().latch().lock();
#elif defined(blinktree_thread_rwlock)
        if constexpr (SHARED)
        {
            node->header().latch().lock_shared();
        }
        else
        {
            node->header().latch().lock();
        }
#endif
    }

    template <bool SHARED> void unlock([[maybe_unused]] Node *node)
    {
#if defined(blinktree_thread_spinlock)
        node->header().latch().unlock();
#elif defined(blinktree_thread_rwlock)
        if constexpr (SHARED)
        {
            node->header().latch().unlock_shared();
        }
        else
        {
            node->header().latch().unlock();
        }
#endif
    }

    void upgrade_lock([[maybe_unused]] Node *node)
    {
#if defined(blinktree_thread_rwlock)
        node->header().latch().lock_upgrade();
#endif
    }
};
} // namespace indices::blinktree::thread::latched