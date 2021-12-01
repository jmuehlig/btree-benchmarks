#pragma once
#include "workload_tuple.h"
#include "workload_type.h"
#include <cstdint>
#include <fstream>
#include <indices/types.h>
#include <ostream>
#include <vector>

namespace benchmark {

class alignas(64) Workload
{
    friend std::ostream &operator<<(std::ostream &stream, const Workload &workload);

public:
    Workload() = default;

    bool operator==(const WorkloadPhase &phase) const { return _phase == phase; }

    inline std::size_t count_fill_inserts() const { return _fill_inserts; }
    inline std::size_t count_benchmark_inserts() const { return _inserts; }
    inline std::size_t count_benchmark_lookups() const { return _lookups; }
    inline std::size_t count_benchmark_updates() const { return _updates; }
    inline std::size_t count_benchmark_workload() const
    {
        return count_benchmark_lookups() + count_benchmark_inserts() + count_benchmark_updates();
    }
    inline std::size_t count_current_phase() const
    {
        return _phase == WorkloadPhase::Fill ? count_fill_inserts() : count_benchmark_workload();
    }
    inline std::size_t count_current_inserts() const
    {
        return _phase == WorkloadPhase::Fill ? count_fill_inserts() : count_benchmark_inserts();
    }
    inline std::size_t count_current_lookups() const
    {
        return _phase == WorkloadPhase::Fill ? 0 : count_benchmark_lookups();
    }
    inline std::size_t count_current_updates() const
    {
        return _phase == WorkloadPhase::Fill ? 0 : count_benchmark_updates();
    }
    inline bool has_benchmark() const { return count_benchmark_workload() > 0; }

    inline const std::vector<WorkloadTuple> &fill_workload() const { return _fill_phase_tuples; }
    inline const std::vector<WorkloadTuple> &benchmark_workload() const { return _workload_phase_tuples; }
    inline std::vector<WorkloadTuple> &fill_workload() { return _fill_phase_tuples; }
    inline std::vector<WorkloadTuple> &benchmark_workload() { return _workload_phase_tuples; }
    inline const std::vector<WorkloadTuple> &current_workload() const
    {
        return _phase == WorkloadPhase::Fill ? fill_workload() : benchmark_workload();
    }
    inline std::vector<WorkloadTuple> &current_workload()
    {
        return _phase == WorkloadPhase::Fill ? fill_workload() : benchmark_workload();
    }

    inline WorkloadPhase phase() const { return _phase; }
    inline void phase(const WorkloadPhase phase) { _phase = phase; }

    void load(const char *fill_file_name, const char *workload_file_name);

private:
    WorkloadPhase _phase = WorkloadPhase::Fill;
    std::size_t _fill_inserts = 0u;
    std::size_t _inserts = 0u;
    std::size_t _lookups = 0u;
    std::size_t _updates = 0u;

    alignas(64) std::vector<WorkloadTuple> _fill_phase_tuples;
    alignas(64) std::vector<WorkloadTuple> _workload_phase_tuples;

    void parse_file(std::ifstream &in_stream, std::vector<WorkloadTuple> &tuples, std::size_t &insert_counter,
                    std::size_t &lookup_counter, std::size_t &update_counter);
};
} // namespace benchmark