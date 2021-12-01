#pragma once

#include "../hardware/builtin.h"
#include <atomic>
#include <cstdint>

namespace util {
class Spinlock
{
public:
    Spinlock() { unlock(); }

    ~Spinlock() = default;

    inline void lock()
    {
        while (true)
        {
            while (_flag.load(std::memory_order_relaxed))
            {
                hardware::Builtin::pause();
            }

            bool expected = false;
            if (_flag.compare_exchange_weak(expected, true, std::memory_order_acquire))
            {
                return;
            }
        }
    }

    inline void unlock() { _flag.store(false, std::memory_order_acquire); }

    inline bool is_locked() const { return _flag.load(std::memory_order_relaxed); }

private:
    std::atomic_bool _flag;
};
} // namespace util
