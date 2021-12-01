#pragma once
#include <array>
#include <atomic>
#include <config.h>
#include <cstdint>
#include <hardware/cache.h>
#include <indices/types.h>
#include <limits>
#include <util/queue_item.h>

#if defined(blinktree_coro_mutex) || defined(blinktree_coro_olfit)
#include <cppcoro/task.hpp>
#endif

#if defined(blinktree_thread_spinlock)
#include "thread/latched/spinlocked_node_base.h"
using NodeBase = indices::blinktree::thread::latched::SpinlockedNodeBase;
#elif defined(blinktree_thread_rwlock)
#include "thread/latched/rw_locked_node_base.h"
using NodeBase = indices::blinktree::thread::latched::RWLockedNodeBase;
#elif defined(blinktree_thread_olfit)
#include "thread/optimistic/optimistic_node_base.h"
using NodeBase = indices::blinktree::thread::optimistic::OptimisticNodeBase;
#elif defined(blinktree_tbb_lock)
#include "tbb/queuing_lock_node_base.h"
using NodeBase = indices::blinktree::tbb::QueuingLockNodeBase;
#elif defined(blinktree_tbb_rw_lock) || defined(blinktree_tbb_no_lock)
#include "tbb/queuing_rw_lock_node_base.h"
using NodeBase = indices::blinktree::tbb::QueuingRWLockNodeBase;
#elif defined(blinktree_tbb_tsx_lock)
#include "tbb/tsx_node_base.h"
using NodeBase = indices::blinktree::tbb::TSXNodeBase;
#elif defined(blinktree_tbb_tsx_rw_lock)
#include "tbb/tsx_rw_node_base.h"
using NodeBase = indices::blinktree::tbb::TSXRWNodeBase;
#elif defined(blinktree_tbb_olfit)
#include "tbb/optimistic_node_base.h"
using NodeBase = indices::blinktree::tbb::OptimisticNodeBase;
#elif defined(blinktree_coro_mutex)
#include "coro/latched/mutexed_node_base.h"
using NodeBase = indices::blinktree::coro::latched::MutexedNodeBase;
#elif defined(blinktree_coro_rw_lock)
#include "coro/latched/rw_locked_node_base.h"
using NodeBase = indices::blinktree::coro::latched::RWLockedNodeBase;
#elif defined(blinktree_coro_olfit)
#include "coro/optimistic/optimistic_node_base.h"
using NodeBase = indices::blinktree::coro::optimistic::OptimisticNodeBase;
#else
class NodeBase
{
};
#endif

#if defined(blinktree_thread_olfit) || defined(blinktree_tbb_olfit) || defined(blinktree_coro_olfit)
#define NODE_GARBAGE_SIZE sizeof(util::QueueItem)
#else
#define NODE_GARBAGE_SIZE 0
#endif

using namespace indices;

namespace indices::blinktree {

class Node;

struct NodeHeader : NodeBase
{
    const bool is_leaf;
    key_type high_key = std::numeric_limits<key_type>::max();
    Node *right_sibling = nullptr;
    alignas(8) std::atomic<Node *> parent;
    size_type size = 0;

    NodeHeader(const bool is_leaf, Node *parent_) : is_leaf(is_leaf), parent(parent_) {}
} __attribute__((packed));

struct InnerNode
{
    static constexpr size_type max_keys =
        (Config::node_size() - NODE_GARBAGE_SIZE - sizeof(NodeHeader) - sizeof(Node *)) /
        (sizeof(key_type) + sizeof(Node *));

    static constexpr size_type max_separators = max_keys + 1;

    std::array<key_type, InnerNode::max_keys> keys;
    std::array<Node *, InnerNode::max_separators> separators;
};

struct LeafNode
{
    static constexpr size_type max_items =
        (Config::node_size() - NODE_GARBAGE_SIZE - sizeof(NodeHeader)) / (sizeof(key_type) + sizeof(value_type));

    std::array<key_type, LeafNode::max_items> keys;
    std::array<value_type, LeafNode::max_items> values;
};

#if defined(blinktree_thread_olfit) || defined(blinktree_tbb_olfit) || defined(blinktree_coro_olfit)
class Node final : public util::QueueItem
#else
class Node
#endif
{
public:
    Node(const bool is_leaf, Node *parent);

#if defined(blinktree_thread_olfit) || defined(blinktree_tbb_olfit) || defined(blinktree_coro_olfit)
    ~Node() override;
#else
    ~Node();
#endif

    inline NodeHeader &header() { return _header; }

    inline bool is_leaf() const { return _header.is_leaf; }
    inline bool is_inner() const { return _header.is_leaf == false; }

    inline size_type size() const { return _header.size; }
    inline void size(const size_type size) { _header.size = size; }

    inline key_type high_key() const { return _header.high_key; }
    inline void high_key(const key_type high_key) { _header.high_key = high_key; }

    inline Node *right_sibling() { return _header.right_sibling; }
    inline void right_sibling(Node *right_sibling) { _header.right_sibling = right_sibling; }

    inline Node *parent() { return _header.parent; }
    inline void parent(Node *parent) { _header.parent = parent; }

    inline value_type value(const size_type index) const { return _leaf_node.values[index]; }
    inline void value(const size_type index, const value_type value) { _leaf_node.values[index] = value; }
    inline value_type *values() { return _leaf_node.values.data(); }

    inline Node *separator(const size_type index) { return _inner_node.separators[index]; }
    inline void separator(const size_type index, Node *separator) { _inner_node.separators[index] = separator; }

    inline key_type leaf_key(const size_type index) const { return _leaf_node.keys[index]; }
    inline key_type inner_key(const size_type index) const { return _inner_node.keys[index]; }
    inline key_type key(const size_type index) const { return is_leaf() ? leaf_key(index) : inner_key(index); }

    inline key_type *leaf_keys() { return _leaf_node.keys.data(); }
    inline key_type *inner_keys() { return _inner_node.keys.data(); }

    inline bool is_full() const
    {
        const size_type max_size = is_leaf() ? LeafNode::max_items : InnerNode::max_keys;
        return size() >= max_size;
    }

    size_type index(const key_type key) const;
    Node *child(const key_type key) const;
#if defined(blinktree_coro_mutex) || defined(blinktree_coro_rw_lock) || defined(blinktree_coro_olfit)
    cppcoro::task<Node *> await_child(const key_type key) const;
#endif

    void insert_separator(const size_type index, Node *separator, const key_type key);
    void insert_value(const size_type index, const value_type value, const key_type key);

    void copy(Node *other, const size_type from_index, const size_type count);

    bool contains_separator(Node *separator) const;

    void prefetch_inner_value(const size_t index) const
    {
        hardware::cache::prefetch<hardware::cache::LLC>((void *)&this->_inner_node.separators[index]);
    }

    void prefetch_leaf_value(const size_t index) const
    {
        hardware::cache::prefetch<hardware::cache::LLC>((void *)&this->_leaf_node.values[index]);
    }

private:
    NodeHeader _header;
    union {
        InnerNode _inner_node;
        LeafNode _leaf_node;
    };
};
} // namespace indices::blinktree