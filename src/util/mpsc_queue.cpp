#include "mpsc_queue.h"

using namespace util;

QueueItem *MPSCQueue::pop_front() noexcept
{
    auto *tail = this->_tail;
    auto *next = tail->next();

    if (tail == this->_end)
    {
        if (next == nullptr)
        {
            return nullptr;
        }

        this->_tail = next;
        tail = next;
        next = next->next();
    }

    if (next != nullptr)
    {
        this->_tail = next;
        return tail;
    }

    const auto *head = this->_head;
    if (tail != head)
    {
        return nullptr;
    }

    this->push_back(this->_end);

    next = tail->next();
    if (next != nullptr)
    {
        this->_tail = next;
        return tail;
    }

    return nullptr;
}