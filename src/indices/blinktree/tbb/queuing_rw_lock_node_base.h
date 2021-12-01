#pragma once

#include <tbb/queuing_rw_mutex.h>

namespace indices::blinktree::tbb {
class QueuingRWLockNodeBase
{
public:
    inline ::tbb::queuing_rw_mutex &latch() { return _latch; }

private:
    ::tbb::queuing_rw_mutex _latch;
};
} // namespace indices::blinktree::tbb