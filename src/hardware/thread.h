#pragma once
#include "system.h"
#include <iostream>
#include <sched.h>
#include <thread>

namespace hardware {
class Thread
{
public:
    static bool pin(std::thread &thread, const std::uint16_t core_id)
    {
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(System::physical_core_id(core_id), &cpuset);
        return pin(thread, cpuset);
    }

    static bool pin(std::thread &thread, const std::vector<std::uint16_t> &core_ids)
    {
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        for (const auto core_id : core_ids)
        {
            CPU_SET(System::physical_core_id(core_id), &cpuset);
        }
        return pin(thread, cpuset);
    }

    static bool pin(const std::uint16_t core_id)
    {
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(System::physical_core_id(core_id), &cpuset);
        return sched_setaffinity(0, sizeof(cpuset), &cpuset) == 0;
    }

private:
    static bool pin(std::thread &thread, const cpu_set_t &cpu_set)
    {
        if (pthread_setaffinity_np(thread.native_handle(), sizeof(cpu_set_t), &cpu_set) != 0)
        {
            std::cerr << "Can not pin thread!" << std::endl;
            return false;
        }

        return true;
    }
};
} // namespace hardware