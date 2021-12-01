#pragma once

#include "core_set.h"
#include <cstdint>
#include <hardware/system.h>
#include <ostream>
#include <vector>

namespace util {
class CoreConfiguration
{
    friend std::ostream &operator<<(std::ostream &stream, const CoreConfiguration &core_configuration);

public:
    enum Type
    {
        Logical,
        NumaAscending
    };
    CoreConfiguration(const Type type, const std::uint16_t min_cores, const std::uint16_t max_cores,
                      const std::uint16_t core_steps = 1);

    operator bool() const { return _core_sets.empty() == false && _index < _core_sets.size(); }

    inline const CoreSet &current() const { return _core_sets[_index]; }
    inline const std::vector<CoreSet> &core_sets() const { return _core_sets; }

    inline void reset() { _index = 0; }

    CoreConfiguration &operator++()
    {
        _index++;
        return *this;
    }

private:
    alignas(64) std::vector<CoreSet> _core_sets;
    alignas(64) std::size_t _index = 0;

    CoreSet generate_core_set(const std::uint16_t count_cores,
                              const std::vector<std::uint16_t> &ordered_physical_cores);
    std::vector<std::uint16_t> generate_core_order(const Type type, const std::uint16_t max_cores);
};
} // namespace util