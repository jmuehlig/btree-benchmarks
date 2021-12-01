#pragma once

#include <tbb/spin_mutex.h>

namespace indices::blinktree::tbb {
class TSXNodeBase
{
public:
    inline ::tbb::speculative_spin_mutex &latch() { return _latch; }

private:
    ::tbb::speculative_spin_mutex _latch;
};
} // namespace indices::blinktree::tbb