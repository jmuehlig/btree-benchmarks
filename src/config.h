#pragma once
#include <cstdint>

class Config
{
public:
    // Node size in Bytes.
    static constexpr auto node_size() { return 1024u; }
};
