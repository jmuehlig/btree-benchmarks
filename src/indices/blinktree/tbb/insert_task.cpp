#include "insert_task.h"
#ifdef blinktree_tbb_olfit
#include <indices/blinktree/epoch_manager.h>
#endif
#include <tbb/queuing_mutex.h>
#include <tbb/queuing_rw_mutex.h>
#include <tbb/spin_rw_mutex.h>

using namespace indices::blinktree::tbb;

InsertTask::InsertTask(TBBTree *tree, const key_type key, const value_type value) : TreeTask(tree, key), _value(value)
{
}

blinktree::Node *InsertTask::execute(blinktree::Node *node)
{
    const auto key = this->key();
#ifdef blinktree_tbb_olfit
    EpochGuard _{this->tree()->epoch_manager()};

    while (true)
    {
        const auto version = node->header().version();
        if (node->high_key() <= key)
        {
            auto right = node->right_sibling();
            if (node->header().is_valid(version))
            {
                return right;
            }
        }
        else if (is_for_leaf() && node->is_inner())
        {
            auto child = node->child(this->key());
            if (node->header().is_valid(version))
            {
                return child;
            }
        }
        else if (node->header().is_valid(version))
        {
            break;
        }
    }

    node->header().acquire();
    if (node->is_leaf())
    {
        auto next = insert_into_leaf_node(node, key, this->_value);
        node->header().release();
        return next;
    }

    auto next = insert_into_inner_node(node, this->_separator_key, this->_separator);
    node->header().release();
    return next;
#else // blinktree_tbb_olfit
#if defined(blinktree_tbb_tsx_lock)
    ::tbb::speculative_spin_mutex::scoped_lock lock(node->header().latch());
#elif defined(blinktree_tbb_tsx_rw_lock)
    ::tbb::speculative_spin_rw_mutex::scoped_lock lock(node->header().latch(), false);
#elif defined(blinktree_tbb_no_lock)
    ::tbb::queuing_rw_mutex::scoped_lock lock(node->header().latch(), false);
#elif defined(blinktree_tbb_rw_lock)
    ::tbb::queuing_rw_mutex::scoped_lock lock(node->header().latch(), false);
#elif defined(blinktree_tbb_lock)
    ::tbb::queuing_mutex::scoped_lock lock(node->header().latch());
#endif

    if (node->high_key() <= key)
    {
        return node->right_sibling();
    }

    // 2) We want to insert into a leaf node.
    if (is_for_leaf())
    {
        if (node->is_inner())
        {
            return node->child(this->key());
        }
        else
        {
#if defined(blinktree_tbb_rw_lock) || defined(blinktree_tbb_no_lock) || defined(blinktree_tbb_tsx_rw_lock)
            lock.upgrade_to_writer();
#endif
            auto next = insert_into_leaf_node(node, key, this->_value);
            return next;
        }
    }

    // 3) We want to insert into an inner node.
#if defined(blinktree_tbb_rw_lock) || defined(blinktree_tbb_no_lock) || defined(blinktree_tbb_tsx_rw_lock)
    lock.upgrade_to_writer();
#endif

    auto next = insert_into_inner_node(node, this->_separator_key, this->_separator);
    return next;
#endif // blinktree_tbb_olfit
}

blinktree::Node *InsertTask::insert_into_leaf_node(blinktree::Node *node, const key_type key, const value_type value)
{
    const auto index = node->index(key);
    if (index < node->size() && node->leaf_key(index) == key)
    {
        // 1) Key is already inserted.
        return nullptr;
    }
    else if (node->is_full() == false)
    {
        // 2) Insert key.
        node->insert_value(index, value, key);
        return nullptr;
    }
    else
    {
        auto *right_leaf = this->tree()->split_leaf_node(node, key, value);

        this->_separator = right_leaf;
        this->_separator_key = right_leaf->leaf_key(0);
        if (node->parent() == nullptr)
        {
            this->tree()->lock();
            this->tree()->create_new_root(node, this->_separator, this->_separator_key);
            this->tree()->unlock();
            return nullptr;
        }
        else
        {
            return node->parent();
        }
    }
}

blinktree::Node *InsertTask::insert_into_inner_node(blinktree::Node *node, const key_type key,
                                                    blinktree::Node *separator)
{
    if (node->is_full() == false)
    {
        const size_type index = node->index(key);
        node->insert_separator(index, separator, key);
        separator->parent(node);
        return nullptr;
    }
    else
    {
        const auto [next_separator, separator_key] = this->tree()->split_inner_node(node, key, separator);

        this->_separator = next_separator;
        this->_separator_key = separator_key;
        if (node->parent() == nullptr)
        {
            this->tree()->lock();
            assert(node == this->tree()->root());
            this->tree()->create_new_root(node, this->_separator, this->_separator_key);
            this->tree()->unlock();
            return nullptr;
        }
        else
        {
            return node->parent();
        }
    }
}