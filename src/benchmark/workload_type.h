#pragma once

namespace benchmark {
enum WorkloadType
{
    Generated,
    YCSB,
    None
};

enum WorkloadPhase
{
    Fill = 0,
    Benchmark = 1
};
} // namespace benchmark