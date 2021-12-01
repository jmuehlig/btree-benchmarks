#pragma once
#include "node.h"
#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>

namespace indices::blinktree {
template <std::size_t S = 10> class NodePath
{
public:
    inline void push(Node *node) { _data[_counter++] = node; }

    inline Node *pop() { return _data[--_counter]; }

    inline void clear() { _counter = 0; }

    inline bool is_empty() const { return _counter == 0; }

    inline std::size_t size() const { return _counter; }

private:
    std::array<Node *, S> _data;
    std::size_t _counter = 0;
};
} // namespace indices::blinktree