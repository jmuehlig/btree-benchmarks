#include "consistency_checker.h"
#include <iostream>

using namespace indices::blinktree;
using namespace indices;

void ConsistencyChecker::check_node_and_print_errors(Node *node, std::ostream &stream) const
{
    if (this->is_high_key_valid(node) == false)
    {
        stream << "Wrong high_key at node " << node << std::endl;
    }

    if (this->is_key_order_valid(node) == false)
    {
        stream << "Wrong key order at node " << node << std::endl;
    }

    if (this->is_no_null_separator(node) == false)
    {
        stream << "Found null separator at node " << node << std::endl;
    }

    if (this->is_children_order_valid(node) == false)
    {
        stream << "Wrong children at node " << node << std::endl;
    }

    if (this->is_level_valid(node) == false)
    {
        stream << "Wrong level at node " << node << std::endl;
    }

    this->check_and_print_parent(node, stream);
}

bool ConsistencyChecker::is_high_key_valid(Node *node) const
{
    if (node->is_leaf())
    {
        return node->leaf_key(node->size() - 1) < node->high_key();
    }
    else
    {
        return node->inner_key(node->size() - 1) < node->high_key();
    }
}

bool ConsistencyChecker::is_key_order_valid(Node *node) const
{
    for (auto index = 1u; index < node->size(); index++)
    {
        if (node->is_leaf())
        {
            if (node->leaf_key(index - 1) >= node->leaf_key(index))
            {
                return false;
            }
        }
        else
        {
            if (node->inner_key(index - 1) >= node->inner_key(index))
            {
                return false;
            }
        }
    }

    return true;
}

bool ConsistencyChecker::is_no_null_separator(Node *node) const
{
    if (node->is_inner())
    {
        for (size_type index = 0; index <= node->size(); index++)
        {
            if (node->separator(index) == nullptr)
            {
                return false;
            }
        }
    }

    return true;
}

bool ConsistencyChecker::is_children_order_valid(Node *node) const
{
    if (node->is_inner())
    {
        for (auto index = 0u; index < node->size(); index++)
        {
            auto *child = node->separator(index);
            const auto child_last_key =
                child->is_leaf() ? child->leaf_key(child->size() - 1) : child->inner_key(child->size() - 1);
            if (child_last_key >= node->inner_key(index))
            {
                return false;
            }
        }
    }

    return true;
}

bool ConsistencyChecker::is_level_valid(blinktree::Node *node) const
{
    if (node->right_sibling() && node->is_leaf() != node->right_sibling()->is_leaf())
    {
        return false;
    }

    if (node->is_inner())
    {
        for (auto index = 0u; index < node->size(); index++)
        {
            if (node->separator(index)->is_leaf() != node->separator(index + 1)->is_leaf())
            {
                return false;
            }
        }
    }

    return true;
}

void ConsistencyChecker::check_and_print_parent(blinktree::Node *node, std::ostream &stream) const
{
    const auto parent = node->parent();
    if (parent)
    {
        if (parent->contains_separator(node) == false)
        {
            stream << "Wrong parent(1) for node " << node << std::endl;
        }
        else
        {
            auto index = 0u;
            for (; index <= parent->size(); index++)
            {
                if (parent->separator(index) == node)
                {
                    break;
                }
            }

            if (index < parent->size())
            {
                if ((node->key(node->size() - 1) < parent->inner_key(index)) == false)
                {
                    stream << "Wrong parent(2) for node " << node << std::endl;
                }
            }
            else
            {
                if ((node->key(0) >= parent->inner_key(index - 1)) == false)
                {
                    stream << "Wrong parent(3) for node " << node << std::endl;
                }
            }
        }
    }
}