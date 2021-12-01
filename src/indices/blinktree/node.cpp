#include "node.h"
#include <algorithm>
#include <cstring>
#include <hardware/builtin.h>
#include <hardware/system.h>
#include <iostream>
#include <limits>

using namespace indices::blinktree;
using namespace indices;

Node::Node(const bool is_leaf, Node *parent) : _header(is_leaf, parent)
{
    static_assert(sizeof(Node) <= Config::node_size());
}

Node::~Node()
{
    if (this->_header.is_leaf == false)
    {
        for (auto i = 0u; i <= this->_header.size; i++)
        {
            if (this->_inner_node.separators[i] != nullptr)
            {
                delete this->_inner_node.separators[i];
            }
        }
    }
}

size_type Node::index(const key_type key) const
{
    const auto keys = this->is_leaf() ? this->_leaf_node.keys.cbegin() : this->_inner_node.keys.cbegin();
    const auto iterator = std::lower_bound(keys, keys + size(), key);

    return std::distance(keys, iterator);
}

Node *Node::child(const key_type key) const
{
    std::int16_t low = 0, high = size() - 1;
    while (low <= high)
    {
        const std::int16_t mid = (low + high) >> 1; // Will work for size() - 1 < MAX_INT/2
        if (this->inner_key(mid) <= key)
        {
            low = mid + 1;
        }
        else
        {
            high = mid - 1;
        }
    }

    return this->_inner_node.separators[high + 1];
}

#if defined(blinktree_coro_mutex) || defined(blinktree_coro_rw_lock) || defined(blinktree_coro_olfit)
#include <indices/blinktree/coro/suspend_always.h>
cppcoro::task<Node *> Node::await_child(const key_type key) const
{
    std::int16_t low = 0, high = size() - 1;
    while (low <= high)
    {
        const std::int16_t mid = (low + high) >> 1; // Will work for size() - 1 < MAX_INT/2
        if (this->inner_key(mid) <= key)
        {
            low = mid + 1;
        }
        else
        {
            high = mid - 1;
        }
    }

    hardware::cache::prefetch<hardware::cache::LLC>((void *)&this->_inner_node.separators[high + 1]);
    co_await coro::suspend_always();

    co_return this->_inner_node.separators[high + 1];
}
#endif

void Node::insert_separator(const size_type index, Node *separator, const key_type key)
{
    if (index < size())
    {
        const auto offset = size() - index;
        std::memmove(&this->_inner_node.keys[index + 1], &this->_inner_node.keys[index], offset * sizeof(key_type));
        std::memmove((void *)&(this->_inner_node.separators[index + 2]),
                     (void *)&(this->_inner_node.separators[index + 1]), offset * sizeof(Node *));
    }

    this->_inner_node.keys[index] = key;
    this->_inner_node.separators[index + 1] = separator;
    this->_header.size++;
}

void Node::insert_value(const size_type index, const value_type value, const key_type key)
{
    if (index < this->size())
    {
        const auto offset = this->size() - index;
        std::memmove(&this->_leaf_node.keys[index + 1], &this->_leaf_node.keys[index], offset * sizeof(key_type));
        std::memmove(&this->_leaf_node.values[index + 1], &this->_leaf_node.values[index], offset * sizeof(value_type));
    }

    this->_leaf_node.keys[index] = key;
    this->_leaf_node.values[index] = value;

    this->_header.size++;
}

void Node::copy(Node *other, const size_type from_index, const size_type count)
{
    if (this->is_leaf())
    {
        std::memcpy(&other->_leaf_node.keys[0], &this->_leaf_node.keys[from_index], count * sizeof(key_type));
        std::memcpy(&other->_leaf_node.values[0], &this->_leaf_node.values[from_index], count * sizeof(value_type));
    }
    else
    {
        std::memcpy(&other->_inner_node.keys[0], &this->_inner_node.keys[from_index], count * sizeof(key_type));
        std::memcpy((void *)&(other->_inner_node.separators[1]),
                    (void *)&(this->_inner_node.separators[from_index + 1]), count * sizeof(Node *));
    }
}

bool Node::contains_separator(Node *separator) const
{
    for (auto i = 0u; i <= this->size(); i++)
    {
        if (this->_inner_node.separators[i] == separator)
        {
            return true;
        }
    }

    return false;
}