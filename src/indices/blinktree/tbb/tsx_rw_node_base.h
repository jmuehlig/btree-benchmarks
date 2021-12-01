#pragma once

#include <tbb/spin_rw_mutex.h>

namespace indices::blinktree::tbb {
class TSXRWNodeBase
{
public:
    inline ::tbb::speculative_spin_rw_mutex &latch() { return _latch; }

private:
    ::tbb::speculative_spin_rw_mutex _latch;
};
} // namespace indices::blinktree::tbb