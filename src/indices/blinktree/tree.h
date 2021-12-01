#pragma once

#include "node.h"
#include <assert.h>
#include <atomic>
#include <cstdlib>
#include <unordered_map>
#include <utility>
#include <vector>

namespace indices::blinktree {
class TreeIterator
{
public:
    TreeIterator(Node *node) : _current_node(node), _first_node_in_level(node) {}

    Node *operator*() { return _current_node; }

    TreeIterator &operator++()
    {
        if ((_current_node)->right_sibling())
        {
            _current_node = (_current_node->right_sibling());
        }
        else if ((_current_node)->is_inner())
        {
            _first_node_in_level = ((_first_node_in_level)->separator(0));
            _current_node = _first_node_in_level;
        }
        else
        {
            _current_node = nullptr;
        }

        return *this;
    }

    bool operator!=(const TreeIterator &other) const { return _current_node != other._current_node; }

private:
    Node *_current_node;
    Node *_first_node_in_level;
};

class Tree
{
public:
    Tree();
    Tree(Node *root_node, size_type height);
    virtual ~Tree();

    inline Node *root() { return _root_node.load(); }

    inline size_type height() const { return _height; }

    inline bool empty() const { return _root_node.load() == nullptr || _root_node.load()->size() == 0; }

    inline Node *create_inner_node(Node *parent) { return create_node(false, parent); }

    inline Node *create_leaf_node(Node *parent) { return create_node(true, parent); }

    void create_new_root(Node *left, Node *right, const key_type key);

    std::pair<Node *, key_type> split_inner_node(Node *inner_node, const key_type key, Node *separator);

    Node *split_leaf_node(Node *leaf_node, const key_type key, const value_type value);

    TreeIterator begin() { return TreeIterator(_root_node); }
    TreeIterator end() { return TreeIterator(nullptr); }

    void check();
    void print_statistics();

protected:
    alignas(64) std::atomic<Node *> _root_node;
    size_type _height = 1;

    virtual Node *create_node(const bool is_leaf, Node *parent);
};
} // namespace indices::blinktree