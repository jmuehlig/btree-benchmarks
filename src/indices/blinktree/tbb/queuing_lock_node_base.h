#pragma once

#include <tbb/queuing_mutex.h>

namespace indices::blinktree::tbb {
class QueuingLockNodeBase
{
public:
    inline ::tbb::queuing_mutex &latch() { return _latch; }

private:
    ::tbb::queuing_mutex _latch;
};
} // namespace indices::blinktree::tbb