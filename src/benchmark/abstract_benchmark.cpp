#include "abstract_benchmark.h"
#include <cassert>
#include <chrono>
#include <config.h>
#include <ctime>
#include <hardware/system.h>
#include <indices/types.h>
#include <iomanip>
#include <iostream>

using namespace benchmark;

AbstractBenchmark::AbstractBenchmark(const util::CLIArguments &config)
    : _core_config(config.core_configuration_type(), config.min_cores(), config.max_cores(), config.core_steps()),
      _config(config)
{
    this->initialize();
}

void AbstractBenchmark::initialize()
{
    this->_workload.load(this->_config.workload_files()[0].c_str(), this->_config.workload_files()[1].c_str());

    assert(this->_core_config && "No cores configured.");

    if (this->_config.is_use_perf())
    {
        this->_perf.add(util::Perf::INSTRUCTIONS);
        this->_perf.add(util::Perf::CYCLES);
        this->_perf.add(util::Perf::L1_MISSES);
        this->_perf.add(util::Perf::LLC_MISSES);
        this->_perf.add(util::Perf::STALLS_MEM_ANY);
    }

    if (this->_config.out_file_name().empty() == false)
    {
        this->_log_file.open(this->_config.out_file_name(), std::fstream::app);
    }

    std::cout << "core configuration: \n" << this->_core_config << std::flush;
    std::cout << "workload: " << this->_workload << std::endl;
}

AbstractBenchmark::~AbstractBenchmark()
{
    if (this->_log_file.is_open())
    {
        this->_log_file << std::endl;
        this->_log_file.close();
    }
}

void AbstractBenchmark::start_timer()
{
    if (this->_config.is_use_perf())
    {
        _perf.start();
    }
    this->_measure_start = std::chrono::steady_clock::now();
}

void AbstractBenchmark::stop_timer()
{
    if (this->_config.is_use_perf())
    {
        _perf.stop();
    }
    this->_measure_end = std::chrono::steady_clock::now();
}

void AbstractBenchmark::print_current_timer(const std::vector<std::pair<std::string, double>> &additional_metrics)
{
    const auto result = this->throughput_and_time();

    std::stringstream result_stream;
    result_stream.imbue(std::locale("en_US.UTF-8"));

    result_stream << this->_core_config.current().physical_cores().size() << " " << this->_current_iteration + 1 << " "
                  << this->_workload.phase() << " \t" << result.second.count() << " ms"
                  << " \t" << result.first << " op/s";

    if (this->_config.is_use_perf())
    {
        auto const normalize_constant = this->workload().count_current_phase();
        result_stream << "\t" << this->_perf[util::Perf::INSTRUCTIONS] / normalize_constant << " ins/op"
                      << "\t" << this->_perf[util::Perf::CYCLES] / normalize_constant << " cycles/op"
                      << "\t" << this->_perf[util::Perf::L1_MISSES] / normalize_constant << " l1-miss/op"
                      << "\t" << this->_perf[util::Perf::LLC_MISSES] / normalize_constant << " llc-miss/op"
                      << "\t" << this->_perf[util::Perf::STALLS_MEM_ANY] / normalize_constant << " memory-stalls/op";
    }

    for (const auto &metric : additional_metrics)
    {
        result_stream << "\t" << metric.second << " " << metric.first;
    }

    if (this->_log_file.is_open())
    {
        this->_log_file << result_stream.str() << std::endl;
    }

    std::cout << result_stream.str() << std::endl;
}

std::pair<double, std::chrono::milliseconds> AbstractBenchmark::throughput_and_time() const
{
    const auto milliseconds =
        std::chrono::duration_cast<std::chrono::milliseconds>(this->_measure_end - this->_measure_start);
    const auto seconds = milliseconds.count() / 1000.0;
    const auto count_operations = double(this->_workload.count_current_phase());
    const auto throughput = count_operations / seconds;

    return std::make_pair(throughput, milliseconds);
}

FinishedRunStatus AbstractBenchmark::is_finished(const bool use_fill_phase)
{
    if (this->is_fill_phase() && this->_workload.has_benchmark() > 0)
    {
        this->_workload.phase(benchmark::WorkloadPhase::Benchmark);
        return FinishedRunStatus(FinishedRunStatus::NextPhase);
    }
    else if (++this->_current_iteration < this->_config.iterations())
    {
        if (use_fill_phase)
        {
            this->clean_tree();
            this->_workload.phase(benchmark::WorkloadPhase::Fill);
        }
        return FinishedRunStatus(FinishedRunStatus::NextRun);
    }
    else if (++this->_core_config)
    {
        this->_current_iteration = 0;
        if (use_fill_phase)
        {
            this->_workload.phase(benchmark::WorkloadPhase::Fill);
            this->clean_tree();
        }
        return FinishedRunStatus(FinishedRunStatus::NextCoreConfiguration);
    }

    this->_current_iteration = 0;
    if (use_fill_phase)
    {
        this->_workload.phase(benchmark::WorkloadPhase::Fill);
        this->clean_tree();
    }
    return FinishedRunStatus(FinishedRunStatus::Finished);
}