#pragma once
#include "tree_task.h"

namespace indices::blinktree::tbb {
class LookupTask : public TreeTask
{
public:
    LookupTask(TBBTree *tree, const key_type key);

    virtual Node *execute(Node *node);

private:
    value_type _value = 0;
};
} // namespace indices::blinktree::tbb