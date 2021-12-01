#include "thread_tree.h"
#include "indices/blinktree/consistency_checker.h"
#include <iostream>

using namespace indices::blinktree;
using namespace indices::blinktree::thread::latched;

void ThreadTree::insert(const key_type key, const value_type value)
{
    NodePath<> path;
    auto *leaf = this->locate_leaf(key, &path);

    // Insert into leaf
    this->upgrade_lock(leaf);
    std::pair<Node *, key_type> up = this->insert_into_leaf(leaf, key, value);
    this->unlock<false>(leaf);

    // Propagate up.
    while (up.first && path.is_empty() == false)
    {
        auto *parent = path.pop();
        this->lock<true>(parent);
        while (parent->high_key() <= up.second)
        {
            auto *right = parent->right_sibling();
            this->unlock<true>(parent);
            parent = right;
            this->lock<true>(parent);
        }
        this->upgrade_lock(parent);
        up = this->insert_into_inner(parent, up.second, up.first);
        this->unlock<false>(parent);
    }

    // Create new root
    if (up.first)
    {
        this->_lock.lock();
        this->create_new_root(this->root(), up.first, up.second);
        this->_lock.unlock();
    }
}

void ThreadTree::update(const indices::key_type key, const indices::value_type value)
{
    NodePath<> path;
    auto *leaf = this->locate_leaf(key, &path);

    const auto index = leaf->index(key);
    if (leaf->key(index) == key)
    {
        this->upgrade_lock(leaf);
        leaf->value(index, value);
        this->unlock<false>(leaf);
    }
    else
    {
        this->unlock<true>(leaf);
    }
}

value_type ThreadTree::find(const key_type key)
{
    auto *leaf = this->locate_leaf(key);

    const auto index = leaf->index(key);
    const auto found_key = leaf->leaf_key(index);
    if (found_key == key)
    {
        const value_type value = leaf->value(index);
        this->unlock<true>(leaf);
        return value;
    }

    this->unlock<true>(leaf);
    return 0;
}

Node *ThreadTree::locate_leaf(const key_type key, NodePath<> *node_path)
{
    auto *current_node = this->root();

    while (current_node->is_inner())
    {
        this->lock<true>(current_node);
        while (current_node->high_key() <= key)
        {
            auto right = current_node->right_sibling();
            this->unlock<true>(current_node);

            current_node = right;
            this->lock<true>(current_node);
        }

        if (node_path)
        {
            node_path->push(current_node);
        }

        auto child = current_node->child(key);
        this->unlock<true>(current_node);
        current_node = child;
    }

    this->lock<true>(current_node);
    while (current_node->high_key() <= key)
    {
        auto right = current_node->right_sibling();
        this->unlock<true>(current_node);

        current_node = right;
        this->lock<true>(current_node);
    }

    return current_node;
}

std::pair<Node *, key_type> ThreadTree::insert_into_leaf(Node *leaf_node, const key_type key, const value_type value)
{
    size_type index = leaf_node->index(key);
    if (index < leaf_node->size() && leaf_node->leaf_key(index) == key)
    {
        return {nullptr, 0};
    }
    else if (leaf_node->is_full() == false)
    {
        leaf_node->insert_value(index, value, key);
        return {nullptr, 0};
    }
    else
    {
        auto right_leaf = this->split_leaf_node(leaf_node, key, value);

        return {right_leaf, right_leaf->leaf_key(0)};
    }
}

std::pair<Node *, key_type> ThreadTree::insert_into_inner(Node *inner_node, const key_type key, Node *separator)
{
    if (inner_node->is_full() == false)
    {
        const size_type index = inner_node->index(key);
        inner_node->insert_separator(index, separator, key);
        return {nullptr, 0};
    }
    else
    {
        return this->split_inner_node(inner_node, key, separator);
    }
}