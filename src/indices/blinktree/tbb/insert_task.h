#pragma once
#include "../node.h"
#include "../node_path.h"
#include "tree_task.h"

namespace indices::blinktree::tbb {
class InsertTask : public TreeTask
{
public:
    InsertTask(TBBTree *tree, const key_type key, const value_type value);

    virtual Node *execute(Node *node);

private:
    const value_type _value;

    // Key that should be inserted into the inner node.
    key_type _separator_key = 0;

    // Link to node that should be inserted into the inner node.
    Node *_separator = nullptr;

    // True, if we are looking for a leaf to insert the (key, value) pair.
    inline bool is_for_leaf() const { return _separator == nullptr; }

    Node *insert_into_leaf_node(Node *node, const key_type key, const value_type value);
    Node *insert_into_inner_node(Node *node, const key_type key, Node *separator);
};
} // namespace indices::blinktree::tbb