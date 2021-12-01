#pragma once

#include "node.h"
#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <indices/types.h>
#include <thread>
#include <util/aligned_t.h>
#include <util/mpsc_queue.h>

namespace indices::blinktree {
class EpochManager
{
public:
    inline static constexpr auto epoch_interval_in_ms = 50u;

    EpochManager(const std::uint16_t count_threads)
        : _count_threads(count_threads),
          _local_epochs(static_cast<util::aligned_t<std::atomic<epoch_t>> *>(
              std::aligned_alloc(64u, sizeof(util::aligned_t<std::atomic<epoch_t>>) * count_threads)))
    {
        for (auto i = 0u; i < count_threads; ++i)
        {
            _local_epochs[i].value().store(0u);
        }
    }

    ~EpochManager();

    void start() noexcept
    {
        _registered_threads = 0u;
        _epoch_thread = new std::thread(&EpochManager::enter_epoch_periodically, this);
        _is_running = true;
    }

    void stop() noexcept;

    void join_epoch() noexcept { _local_epochs[_thread_id].value().store(_global_epoch.load()); }
    void leave_epoch() noexcept { _local_epochs[_thread_id].value().store(std::numeric_limits<epoch_t>::max()); }

    void add_to_garbage(Node *garbage) noexcept
    {
        garbage->header().remove_epoch(_global_epoch.load(std::memory_order_relaxed));
        _garbage_queue.push_back(garbage);
    }

    void enter_epoch_periodically();

    void register_thread() noexcept { _thread_id = _registered_threads.fetch_add(1u); }

private:
    inline static thread_local std::uint16_t _thread_id{std::numeric_limits<std::uint16_t>::max()};
    const std::uint16_t _count_threads;
    std::atomic<std::uint16_t> _registered_threads{0u};
    util::aligned_t<std::atomic<epoch_t>> *_local_epochs;
    alignas(64) std::atomic<epoch_t> _global_epoch{0u};
    alignas(64) std::atomic_bool _is_running{false};
    std::thread *_epoch_thread;
    alignas(64) util::MPSCQueue _garbage_queue;

    void reclaim_epoch_garbage();

    epoch_t min_local_epoch() const noexcept;
};

class EpochGuard
{
public:
    explicit EpochGuard(EpochManager &epoch_manager) noexcept : _epoch_manager(epoch_manager)
    {
        _epoch_manager.join_epoch();
    }

    ~EpochGuard() noexcept { _epoch_manager.leave_epoch(); }

private:
    EpochManager &_epoch_manager;
};
} // namespace indices::blinktree