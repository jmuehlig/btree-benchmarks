#include "tree_statistics.h"
#include "config.h"
#include <iostream>

using namespace indices::blinktree;

void TreeStatistics::collect_node_statistics(Node *node)
{
    this->_count_inner_nodes += node->is_inner();
    this->_count_leaf_nodes += node->is_leaf();
    this->_count_inner_node_keys += node->is_inner() * node->size();
    this->_count_leaf_node_keys += node->is_leaf() * node->size();
}