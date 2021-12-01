#pragma once
#include "tree_task.h"

namespace indices::blinktree::tbb {
class UpdateTask : public TreeTask
{
public:
    UpdateTask(TBBTree *tree, const key_type key, const value_type value);

    virtual Node *execute(Node *node);

private:
    value_type _value = 0;
};
} // namespace indices::blinktree::tbb