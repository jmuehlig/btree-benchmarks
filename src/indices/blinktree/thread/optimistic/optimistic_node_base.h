#pragma once
#include "indices/blinktree/optimistic_lock.h"
#include <atomic>
#include <cstdint>
#include <hardware/builtin.h>
#include <util/spinlock.h>

namespace indices::blinktree::thread::optimistic {
class OptimisticNodeBase
{
public:
    OptimisticNodeBase() = default;

    inline OptimisticLock::version_t version() const { return _optimistic_latch.read_valid(); }
    inline bool is_valid(const OptimisticLock::version_t version) const { return _optimistic_latch.is_valid(version); }

    inline void acquire() noexcept { _optimistic_latch.lock(); }
    inline void release() noexcept { _optimistic_latch.unlock(); }

    inline epoch_t remove_epoch() const { return _remove_epoch; }
    inline void remove_epoch(const epoch_t remove_epoch) { _remove_epoch = remove_epoch; }

private:
    epoch_t _remove_epoch{0u};
    OptimisticLock _optimistic_latch;
};
} // namespace indices::blinktree::thread::optimistic