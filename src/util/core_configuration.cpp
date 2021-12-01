#include "core_configuration.h"
#include <algorithm>
#include <cassert>
#include <config.h>
#include <hardware/system.h>
#include <iostream>
#include <map>
#include <numeric>

using namespace util;

CoreConfiguration::CoreConfiguration(const Type type, std::uint16_t min_cores, const std::uint16_t max_cores,
                                     const std::uint16_t core_step)
{
    assert(core_step > 0);

    const std::vector<std::uint16_t> ordered_cores = this->generate_core_order(type, max_cores);

    if (min_cores == 0 || min_cores == max_cores)
    {
        // Just one core config with max_cores.
        this->_core_sets.push_back(this->generate_core_set(max_cores, ordered_cores));
    }
    else
    {
        // Start with one core.
        if (min_cores % core_step != 0)
        {
            this->_core_sets.push_back(this->generate_core_set(min_cores, ordered_cores));
            min_cores++;
        }

        // Increase by core_step until max_cores.
        for (auto count_cores = min_cores; count_cores <= max_cores; count_cores++)
        {
            if (count_cores % core_step == 0)
            {
                this->_core_sets.push_back(this->generate_core_set(count_cores, ordered_cores));
            }
        }
        // If not included, end with max_cores.
        if (max_cores % core_step != 0)
        {
            this->_core_sets.push_back(this->generate_core_set(max_cores, ordered_cores));
        }
    }
}

CoreSet CoreConfiguration::generate_core_set(const std::uint16_t count_cores,
                                             const std::vector<std::uint16_t> &ordered_physical_cores)
{
    assert(count_cores <= ordered_physical_cores.size());

    std::vector<std::uint16_t> physical_cores;
    for (auto core_index = 0u; core_index < count_cores; core_index++)
    {
        const auto physical_core = ordered_physical_cores[core_index];
        physical_cores.push_back(physical_core);
    }

    auto core_set = CoreSet(physical_cores);
    assert(core_set == true);

    return core_set;
}

std::vector<std::uint16_t> CoreConfiguration::generate_core_order(const Type type, const std::uint16_t max_cores)
{
    const auto real_max_cores = std::max(max_cores, hardware::System::count_cores());

    std::vector<std::uint16_t> core_order(real_max_cores);
    if (type == Logical)
    {
        std::iota(core_order.begin(), core_order.end(), 0);
    }
    else if (type == NumaAscending)
    {

        std::map<std::uint8_t, std::vector<std::uint16_t>> node_cores;
        for (auto core_id = 0u; core_id < real_max_cores; core_id++)
        {
            const auto numa_id = hardware::System::numa_id(core_id);
            node_cores[numa_id].push_back(core_id);
        }

        std::size_t index = 0;
        for (auto &node : node_cores)
        {
            for (auto core_id : node.second)
            {
                core_order[index++] = core_id;
                if (index == core_order.size())
                {
                    return core_order;
                }
            }
        }
    }

    return core_order;
}

std::ostream &util::operator<<(std::ostream &stream, const CoreConfiguration &core_configuration)
{
    for (auto &core_set : core_configuration.core_sets())
    {
        const auto &cores = core_set.physical_cores();
        stream << " " << cores.size() << ": ";
        auto last_numa_region = hardware::System::numa_id(cores[0]);
        for (auto core_id : cores)
        {
            if (hardware::System::numa_id(core_id) != last_numa_region)
            {
                stream << "| ";
                last_numa_region = hardware::System::numa_id(core_id);
            }

            stream << core_id << " ";
        }
        stream << "\n";
    }

    return stream << std::flush;
}