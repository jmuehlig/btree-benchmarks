#include "thread_tree.h"
#include "indices/blinktree/consistency_checker.h"
#include "optimistic_node_base.h"
#include <hardware/builtin.h>

using namespace indices::blinktree;
using namespace indices::blinktree::thread::optimistic;

ThreadTree::~ThreadTree() noexcept
{
    this->reclaim_node(this->root());
    this->_root_node = nullptr;
}

void ThreadTree::insert(const key_type key, const value_type value)
{
    NodePath<> path;

    EpochGuard _{this->_epoch_manager};

    auto leaf = this->locate_leaf(key, &path);

    do
    {
        auto version = leaf->header().version();
        if (leaf->header().is_valid(version))
        {
            while (key >= leaf->high_key() && leaf->right_sibling())
            {
                auto next = leaf->right_sibling();
                if (leaf->header().is_valid(version))
                {
                    leaf = next;
                }
                version = leaf->header().version();
            }

            leaf->header().acquire();
            auto up = this->insert_into_leaf(leaf, key, value);
            leaf->header().release();

            while (up.first && path.is_empty() == false)
            {
                auto parent = path.pop();
                parent->header().acquire();
                while (parent->high_key() <= up.second)
                {
                    auto right = parent->right_sibling();
                    parent->header().release();
                    parent = right;
                    parent->header().acquire();
                }
                up = this->insert_into_inner(parent, up.second, up.first);
                parent->header().release();
            }

            if (up.first)
            {
                this->_lock.lock();
                this->create_new_root(this->root(), up.first, up.second);
                this->_lock.unlock();
            }
            return;
        }
    } while (true);
}

void ThreadTree::update(const indices::key_type key, const indices::value_type value)
{
    NodePath<> path;

    EpochGuard _{this->_epoch_manager};

    auto leaf = this->locate_leaf(key, &path);

    do
    {
        auto version = leaf->header().version();
        while (key >= leaf->high_key() && leaf->right_sibling())
        {
            auto next = leaf->right_sibling();
            if (leaf->header().is_valid(version))
            {
                leaf = next;
            }
            version = leaf->header().version();
        }

        if (leaf->header().is_valid(version))
        {
            leaf->header().acquire();
            const auto index = leaf->index(key);
            if (leaf->key(index) == key)
            {
                leaf->value(index, value);
            }
            leaf->header().release();
            return;
        }
    } while (true);
}

value_type ThreadTree::find(const key_type key)
{
    EpochGuard _{this->_epoch_manager};

    auto leaf = this->locate_leaf(key);
    do
    {
        auto version = leaf->header().version();
        while (key >= leaf->high_key() && leaf->right_sibling())
        {
            auto next = leaf->right_sibling();
            if (leaf->header().is_valid(version))
            {
                leaf = next;
            }
            version = leaf->header().version();
        }

        const auto index = leaf->index(key);
        const auto found_key = leaf->leaf_key(index);
        const auto value = leaf->value(index);
        if (leaf->header().is_valid(version))
        {
            return found_key == key ? value : 0;
        }
    } while (true);
}

Node *ThreadTree::locate_leaf(const key_type key, NodePath<> *node_path)
{
    auto current = this->root();
    auto version = current->header().version();
    while (current->is_inner())
    {
        auto right = current->right_sibling();
        auto next = current->child(key);
        if (current->header().is_valid(version))
        {
            if (next != right && node_path)
            {
                node_path->push(current);
            }

            current = next;
        }
        version = current->header().version();
    }

    return current;
}

std::pair<Node *, key_type> ThreadTree::insert_into_leaf(Node *leaf_node, const key_type key, const value_type value)
{
    size_type index = leaf_node->index(key);
    if (index < leaf_node->size() && leaf_node->leaf_key(index) == key)
    {
        return std::make_pair(nullptr, 0);
    }
    else if (leaf_node->is_full() == false)
    {
        leaf_node->insert_value(index, value, key);
        return {nullptr, 0};
    }
    else
    {
        auto *right_leaf = this->split_leaf_node(leaf_node, key, value);

        return std::make_pair(right_leaf, right_leaf->leaf_key(0));
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