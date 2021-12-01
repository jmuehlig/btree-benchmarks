#pragma once
#include <ostream>

#include "node.h"

namespace indices::blinktree {
class ConsistencyChecker
{
public:
    void check_node_and_print_errors(Node *node, std::ostream &stream) const;

private:
    bool is_high_key_valid(Node *node) const;
    bool is_key_order_valid(Node *node) const;
    bool is_no_null_separator(Node *node) const;
    bool is_children_order_valid(Node *node) const;
    bool is_level_valid(Node *node) const;

    void check_and_print_parent(Node *node, std::ostream &stream) const;
};
} // namespace indices::blinktree