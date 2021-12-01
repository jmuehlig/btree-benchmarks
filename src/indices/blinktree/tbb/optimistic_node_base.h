#pragma once
#include "indices/blinktree/optimistic_lock.h"
#include "indices/types.h"
#include <atomic>
#include <cstdint>
#include <hardware/builtin.h>
#include <tbb/queuing_rw_mutex.h>

namespace indices::blinktree::tbb {
class OptimisticNodeBase
{
public:
    OptimisticNodeBase() = default;

    inline OptimisticLock::version_t version() const { return _optimistic_latch.read_valid(); }

    inline bool is_valid(const OptimisticLock::version_t version) { return _optimistic_latch.is_valid(version); }

    void acquire() noexcept { _optimistic_latch.lock(); }
    void release() noexcept { _optimistic_latch.unlock(); }

    inline epoch_t remove_epoch() const { return _remove_epoch; }
    void remove_epoch(const epoch_t epoch) { _remove_epoch = epoch; }

private:
    epoch_t _remove_epoch{0u};
    OptimisticLock _optimistic_latch;
};
} // namespace indices::blinktree::tbb