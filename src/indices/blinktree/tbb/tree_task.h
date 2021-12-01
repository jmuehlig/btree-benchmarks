#pragma once

#include "node_task.h"
#include "tbb_tree.h"

namespace indices::blinktree::tbb {
class TreeTask : public NodeTask
{
public:
    TreeTask(TBBTree *tree, const key_type key) : NodeTask(tree->root()), _tree(tree), _key(key) {}

    virtual ~TreeTask() = default;

protected:
    inline TBBTree *tree() const { return _tree; }
    inline key_type key() const { return _key; }

private:
    TBBTree *_tree;
    const key_type _key;
};
} // namespace indices::blinktree::tbb