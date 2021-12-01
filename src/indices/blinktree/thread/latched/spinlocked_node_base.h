#pragma once

#include <util/spinlock.h>

namespace indices::blinktree::thread::latched {
class SpinlockedNodeBase
{
public:
    inline util::Spinlock &latch() { return _latch; }

private:
    util::Spinlock _latch;
};
} // namespace indices::blinktree::thread::latched