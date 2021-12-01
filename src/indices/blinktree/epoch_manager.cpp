#include "epoch_manager.h"
#include <util/queue.h>
using namespace indices::blinktree;

EpochManager::~EpochManager()
{
    Node *garbage;
    while ((garbage = reinterpret_cast<Node *>(this->_garbage_queue.pop_front())) != nullptr)
    {
        std::free(garbage);
    }

    delete _local_epochs;
}

void EpochManager::enter_epoch_periodically()
{
    while (this->_is_running == false)
        ;

    while (this->_is_running)
    {
        this->_global_epoch.fetch_add(1u);
        this->reclaim_epoch_garbage();
        std::this_thread::sleep_for(std::chrono::milliseconds(this->epoch_interval_in_ms));
    }
}

void EpochManager::reclaim_epoch_garbage()
{
    const auto min_epoch = this->min_local_epoch();

    util::Queue deferred_garbage;

    Node *garbage;
    while ((garbage = reinterpret_cast<Node *>(this->_garbage_queue.pop_front())) != nullptr)
    {
        if (garbage->header().remove_epoch() < min_epoch)
        {
            std::free(garbage);
        }
        else
        {
            deferred_garbage.push_back(garbage);
        }
    }

    if (deferred_garbage.empty() == false)
    {
        this->_garbage_queue.push_back(deferred_garbage.begin(), deferred_garbage.end());
    }
}

void EpochManager::stop() noexcept
{
    this->_is_running = false;
    this->_epoch_thread->join();
    delete this->_epoch_thread;

    Node *garbage;
    while ((garbage = reinterpret_cast<Node *>(this->_garbage_queue.pop_front())) != nullptr)
    {
        std::free(garbage);
    }
}

epoch_t EpochManager::min_local_epoch() const noexcept
{
    auto min_epoch = this->_local_epochs[0].value().load();
    for (auto i = 1u; i < this->_count_threads; ++i)
    {
        min_epoch = std::min(min_epoch, this->_local_epochs[i].value().load());
    }

    return min_epoch;
}