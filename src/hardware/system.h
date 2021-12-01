#pragma once

#include <cstdint>
#include <numa.h>
#include <sched.h>
#include <thread>

namespace hardware {
class System
{
public:
    static std::uint16_t core_id() { return std::uint16_t(sched_getcpu()); }

    static std::uint8_t numa_id(const std::uint16_t core_id)
    {
        return std::uint8_t(numa_node_of_cpu(physical_core_id(core_id)));
    }

    static std::uint8_t max_numa_node() { return std::uint8_t(numa_max_node()); }

    static std::uint16_t count_cores() { return std::uint16_t(std::thread::hardware_concurrency()); }

    static std::uint16_t physical_core_id(const std::uint16_t logical_core_id)
    {
        return logical_core_id % count_cores();
    }
};
} // namespace hardware