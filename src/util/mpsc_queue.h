#pragma once

#include "queue_item.h"
#include <atomic>
#include <cstdint>

namespace util {
/**
 * Multi producer, single consumer queue with unlimited slots.
 * Every thread can push values into the queue without using latches.
 *
 * Inspired by http://www.1024cores.net/home/lock-free-algorithms/queues/intrusive-mpsc-node-based-queue
 */
class MPSCQueue
{
public:
    constexpr MPSCQueue() noexcept : _head(&_stub), _tail(&_stub), _end(&_stub) {}
    ~MPSCQueue() noexcept = default;

    /**
     * Inserts the given item into the queue.
     * @param item Item to insert.
     */
    void push_back(QueueItem *item) noexcept
    {
        item->next(nullptr);
        auto *prev = __atomic_exchange_n(&_head, item, __ATOMIC_RELAXED);
        prev->next(item);
    }

    /**
     * Inserts all items between begin and end into the queue.
     * Items must be linked among themselves.
     * @param begin First item to insert.
     * @param end Last item to insert.
     */
    void push_back(QueueItem *begin, QueueItem *end) noexcept
    {
        end->next(nullptr);
        auto *old_head = __atomic_exchange_n(&_head, end, __ATOMIC_RELAXED);
        old_head->next(begin);
    }

    /**
     * @return End of the queue.
     */
    [[nodiscard]] const QueueItem *end() const noexcept { return _end; }

    /**
     * @return True, when the queue is empty.
     */
    [[nodiscard]] bool empty() const noexcept { return _tail == _end && _stub.next() == nullptr; }

    /**
     * @return Takes and removes the first item from the queue.
     */
    QueueItem *pop_front() noexcept;

private:
    // Head of the queue (accessed by every producer).
    alignas(64) QueueItem *_head;

    // Tail of the queue (accessed by the consumer and producers if queue is empty)-
    alignas(64) QueueItem *_tail;

    // Pointer to the end.
    alignas(16) QueueItem *const _end;

    // Dummy item for empty queue.
    alignas(16) QueueItem _stub;
};
} // namespace util