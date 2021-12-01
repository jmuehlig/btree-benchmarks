#pragma once

#include "../node.h"
#include <cassert>
#include <iostream>
#include <tbb/queuing_rw_mutex.h>
#include <tbb/spin_rw_mutex.h>
#include <tbb/task.h>

namespace indices::blinktree::tbb {
class NodeTask : public ::tbb::task
{
public:
    NodeTask(Node *node) : _node(node) {}
    virtual ~NodeTask() = default;

    virtual ::tbb::task *execute()
    {
        auto *next_node = this->execute(this->_node);

        if (next_node != nullptr)
        {
            _node = next_node;
            this->recycle_as_continuation();
            return this;
        }

        return nullptr;
    }

    virtual Node *execute(Node *node) = 0;
    inline void node(Node *node) { _node = node; }

private:
    Node *_node;
};
} // namespace indices::blinktree::tbb