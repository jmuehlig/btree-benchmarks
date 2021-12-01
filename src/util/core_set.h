#pragma once

#include <cassert>
#include <cstdint>
#include <vector>

namespace util {
class CoreSet
{
public:
    CoreSet() = default;
    explicit CoreSet(const std::vector<std::uint16_t> &physical_cores) : _physical_cores(physical_cores) {}

    operator bool() const { return _physical_cores.empty() == false; }

    CoreSet &operator=(const CoreSet &other) = default;

    inline const std::vector<std::uint16_t> &physical_cores() const { return _physical_cores; }

private:
    std::vector<std::uint16_t> _physical_cores;
};
} // namespace util