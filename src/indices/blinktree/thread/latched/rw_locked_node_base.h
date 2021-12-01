#pragma once

#include <util/rw_spinlock.h>

namespace indices::blinktree::thread::latched {
class RWLockedNodeBase
{
public:
    inline util::RWSpinLock &latch() { return _latch; }

private:
    util::RWSpinLock _latch;
};
} // namespace indices::blinktree::thread::latched