#pragma once

#include "../tree.h"
#include <tbb/spin_mutex.h>
#ifdef blinktree_tbb_olfit
#include "indices/blinktree/epoch_manager.h"
#endif

namespace indices::blinktree::tbb {
class TBBTree : public Tree
{
public:
#ifdef blinktree_tbb_olfit
    TBBTree(const std::uint16_t count_threads) : _epoch_manager(count_threads) {}
    ~TBBTree() noexcept override
    {
        this->reclaim_node(this->root());
        this->_root_node = nullptr;
    }
#else
    TBBTree() = default;
    ~TBBTree() noexcept override = default;
#endif

    inline void lock() { _lock.lock(); }

    inline void unlock() { _lock.unlock(); }

#ifdef blinktree_tbb_olfit
    EpochManager &epoch_manager() noexcept { return _epoch_manager; }
    void register_thread() { _epoch_manager.register_thread(); }
    void start_epoch() { _epoch_manager.start(); }
    void stop_epoch() { _epoch_manager.stop(); }
#endif

private:
    ::tbb::spin_mutex _lock;
#ifdef blinktree_tbb_olfit
    alignas(64) indices::blinktree::EpochManager _epoch_manager;

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
#endif
};
} // namespace indices::blinktree::tbb