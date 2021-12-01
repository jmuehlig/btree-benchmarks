#include "lookup_task.h"
#ifdef blinktree_tbb_olfit
#include <indices/blinktree/epoch_manager.h>
#endif
#include <tbb/queuing_mutex.h>
#include <tbb/queuing_rw_mutex.h>
#include <tbb/spin_mutex.h>
#include <tbb/spin_rw_mutex.h>

using namespace indices::blinktree::tbb;

LookupTask::LookupTask(TBBTree *tree, const key_type key) : TreeTask(tree, key)
{
}

blinktree::Node *LookupTask::execute(blinktree::Node *node)
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

        const auto index = node->index(this->key());
        const auto found_key = node->leaf_key(index);
        if (found_key == this->key())
        {
            this->_value = node->value(index);
        }

        if (node->header().is_valid(version))
        {
            return nullptr;
        }
    }
#else // blinktree_tbb_olfit
#if defined(blinktree_tbb_tsx_lock)
    ::tbb::speculative_spin_mutex::scoped_lock lock(node->header().latch());
#elif defined(blinktree_tbb_tsx_rw_lock)
    ::tbb::speculative_spin_rw_mutex::scoped_lock lock(node->header().latch(), false);
#elif defined(blinktree_tbb_no_lock)
    // Dont lock
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
    }

    const auto index = node->index(this->key());
    const auto found_key = node->leaf_key(index);
    if (found_key == this->key())
    {
        this->_value = node->value(index);
    }
    return nullptr;
#endif // blinktree_tbb_olfit
}