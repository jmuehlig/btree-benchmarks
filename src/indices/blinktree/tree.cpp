#include "tree.h"
#include "consistency_checker.h"
#include "tree_statistics.h"
#include <cstdlib>
#include <iostream>
#include <new>

using namespace indices::blinktree;

Tree::Tree() : _root_node(create_node(true, nullptr))
{
}

Tree::~Tree()
{
    delete this->_root_node;
}

blinktree::Node *Tree::create_node(const bool is_leaf, blinktree::Node *parent)
{
    return new (std::aligned_alloc(64u, Config::node_size())) Node(is_leaf, parent);
}

void Tree::create_new_root(Node *left, Node *right, const key_type key)
{
    auto *root = this->create_inner_node(nullptr);

    left->parent(root);
    root->separator(0, left);

    right->parent(root);
    root->insert_separator(0, right, key);

    this->_height++;
    this->_root_node = root;
}

std::pair<blinktree::Node *, key_type> Tree::split_inner_node(Node *inner_node, const key_type key, Node *separator)
{
    constexpr auto left_size = InnerNode::max_keys / 2;
    constexpr auto right_size = InnerNode::max_keys - left_size;

    key_type key_up;
    Node *new_inner_node = this->create_inner_node(inner_node->parent());
    new_inner_node->high_key(inner_node->high_key());

    if (key < inner_node->inner_key(left_size - 1))
    {
        inner_node->copy(new_inner_node, left_size, right_size);
        new_inner_node->separator(0, inner_node->separator(left_size));
        new_inner_node->size(right_size);

        key_up = inner_node->inner_key(left_size - 1);
        inner_node->size(left_size - 1);

        const size_type index = inner_node->index(key);
        separator->parent(inner_node);
        inner_node->insert_separator(index, separator, key);
    }
    else if (key < inner_node->inner_key(left_size))
    {
        inner_node->copy(new_inner_node, left_size, right_size);
        new_inner_node->separator(0, separator);
        key_up = key;
        inner_node->size(left_size);
        new_inner_node->size(right_size);
    }
    else
    {
        inner_node->copy(new_inner_node, left_size + 1, right_size - 1);
        new_inner_node->separator(0, inner_node->separator(left_size + 1));
        inner_node->size(left_size);
        new_inner_node->size(right_size - 1);
        key_up = inner_node->inner_key(left_size);

        const size_type index = new_inner_node->index(key);
        new_inner_node->insert_separator(index, separator, key);
    }

    new_inner_node->right_sibling(inner_node->right_sibling());
    inner_node->right_sibling(new_inner_node);
    inner_node->high_key(key_up);

    for (auto index = 0u; index <= new_inner_node->size(); index++)
    {
        new_inner_node->separator(index)->parent(new_inner_node);
    }

    return std::make_pair(new_inner_node, key_up);
}

blinktree::Node *Tree::split_leaf_node(Node *leaf_node, const key_type key, const value_type value)
{
    constexpr size_type left_size = LeafNode::max_items / 2;
    constexpr size_type right_size = LeafNode::max_items - left_size;

    auto *new_leaf_node = this->create_leaf_node(leaf_node->parent());

    leaf_node->copy(new_leaf_node, left_size, right_size);
    new_leaf_node->high_key(leaf_node->high_key());
    new_leaf_node->right_sibling(leaf_node->right_sibling());
    new_leaf_node->size(right_size);
    leaf_node->size(left_size);
    leaf_node->right_sibling(new_leaf_node);

    if (key < new_leaf_node->leaf_key(0))
    {
        leaf_node->insert_value(leaf_node->index(key), value, key);
    }
    else
    {
        new_leaf_node->insert_value(new_leaf_node->index(key), value, key);
    }

    leaf_node->high_key(new_leaf_node->leaf_key(0));

    return new_leaf_node;
}

void Tree::check()
{
    ConsistencyChecker consistency_checker;

    for (auto node : *this)
    {
        consistency_checker.check_node_and_print_errors(node, std::cerr);
    }
}

void Tree::print_statistics()
{
    TreeStatistics statistics(this);

    for (auto node : *this)
    {
        statistics.collect_node_statistics(node);
    }

    std::cout << statistics << std::endl;
}