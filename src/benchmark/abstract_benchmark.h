#pragma once
#include "workload.h"
#include <chrono>
#include <cstdint>
#include <fstream>
#include <string>
#include <util/cli_arguments.h>
#include <util/core_configuration.h>
#include <util/perf.h>
#include <utility>
#include <vector>

namespace benchmark {
class FinishedRunStatus
{
public:
    enum Status
    {
        NextPhase,
        NextRun,
        NextCoreConfiguration,
        Finished
    };
    FinishedRunStatus(Status status) : _status(status) {}

    bool is_finished() const { return _status == Finished; }
    bool is_next_core_configuration() const { return _status == NextCoreConfiguration; }

    operator bool() const { return is_finished(); }

private:
    Status _status;
};

class alignas(64) AbstractBenchmark
{
public:
    AbstractBenchmark(const util::CLIArguments &configuration);
    virtual ~AbstractBenchmark();

    void start_timer();
    void stop_timer();
    void print_current_timer()
    {
        auto metric = std::vector<std::pair<std::string, double>>();
        print_current_timer(metric);
    }
    void print_current_timer(const std::vector<std::pair<std::string, double>> &additional_metrics);
    std::pair<double, std::chrono::milliseconds> throughput_and_time() const;

    inline Workload &workload() { return _workload; }
    inline const util::CLIArguments &config() const { return _config; }

    template <class V> static inline __attribute__((always_inline)) void DoNotOptimize(V &value)
    {
#if defined(__clang__)
        asm volatile("" : "+r,m"(value) : : "memory");
#else
        asm volatile("" : "+m,r"(value) : : "memory");
#endif
    }

    template <class V> static inline __attribute__((always_inline)) void DoNotOptimize(V const &value)
    {
        asm volatile("" : : "r,m"(value) : "memory");
    }

protected:
    FinishedRunStatus is_finished(const bool use_fill_phase = true);
    inline util::CoreConfiguration &core_config() { return _core_config; }
    virtual void clean_tree() = 0;
    inline std::uint16_t current_iteration() const { return _current_iteration; }
    inline bool is_fill_phase() const { return _workload == WorkloadPhase::Fill; }

private:
    void initialize();

    alignas(64) util::CoreConfiguration _core_config;
    alignas(64) Workload _workload;
    std::uint16_t _current_iteration = 0;

    alignas(64) const util::CLIArguments &_config;

    alignas(64) std::chrono::time_point<std::chrono::steady_clock> _measure_start;
    std::chrono::time_point<std::chrono::steady_clock> _measure_end;

    alignas(64) util::Perf _perf;
    alignas(64) std::ofstream _log_file;
};
} // namespace benchmark