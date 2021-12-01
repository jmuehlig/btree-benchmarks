#include "update_task.h"
#ifdef blinktree_tbb_olfit
#include <indices/blinktree/epoch_manager.h>
#endif
#include <tbb/queuing_mutex.h>
#include <tbb/queuing_rw_mutex.h>
#include <tbb/spin_mutex.h>
#include <tbb/spin_rw_mutex.h>

using namespace indices::blinktree::tbb;

UpdateTask::UpdateTask(TBBTree *tree, const key_type key, const value_type value) : TreeTask(tree, key), _value(value)
{
}

blinktree::Node *UpdateTask::execute(blinktree::Node *node)
{
#ifdef blinktree_tbb_olfit
    EpochGuard _{this->tree()->epoch_manager()};

    while (true)
    {
        const auto version = node->header().version();
        if (node->high_key() <= this->key())
        {
            auto right = node->right_sibling();
            if (node->header().is_valid(version))
            {
                return right;
            }
        }
        else if (node->is_inner())
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
    const auto index = node->index(this->key());
    const auto found_key = node->leaf_key(index);
    if (found_key == this->key())
    {
        node->value(index, this->_value);
    }
    node->header().release();
    return nullptr;
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

    if (node->high_key() <= this->key())
    {
        return node->right_sibling();
    }

    if (node->is_leaf() == false)
    {
        return node->child(this->key());
        ;
    }
    else
    {
#if defined(blinktree_tbb_rw_lock) || defined(blinktree_tbb_no_lock) || defined(blinktree_tbb_tsx_rw_lock)
        lock.upgrade_to_writer();
#endif
        const auto index = node->index(this->key());
        const auto found_key = node->leaf_key(index);
        if (found_key == this->key())
        {
            node->value(index, this->_value);
        }
        return nullptr;
    }
#endif // blinktree_tbb_olfit
}