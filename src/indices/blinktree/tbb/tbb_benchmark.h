#pragma once

#include "tbb/task.h"
#include "tbb_tree.h"
#include <array>
#include <benchmark/abstract_benchmark.h>
#include <benchmark/workload_tuple.h>
#include <config.h>
#include <cstdint>
#include <hardware/thread.h>
#include <iostream>
#include <tbb/task_scheduler_observer.h>
#include <thread>
#include <vector>

namespace indices::blinktree::tbb {
class TBBBenchmark : public benchmark::AbstractBenchmark
{
public:
    explicit TBBBenchmark(const util::CLIArguments &config);
    virtual ~TBBBenchmark() = default;

    void execute();

protected:
    virtual void clean_tree() { delete _tree; }

private:
    TBBTree *_tree = nullptr;

    void check_and_print_tree();
};

class SchedulerObserver : public ::tbb::task_scheduler_observer
{
public:
    SchedulerObserver(const std::vector<std::uint16_t> &cores, TBBTree *tree)
        : ::tbb::task_scheduler_observer(), _cores(cores), _tree(tree)
    {
    }

    virtual ~SchedulerObserver() = default;

    void on_scheduler_entry(bool) override
    {
        const auto worker_id = ::tbb::this_task_arena::current_thread_index();
        const auto core_id = _cores[worker_id];
        hardware::Thread::pin(core_id);
#ifdef blinktree_tbb_olfit
        this->_tree->register_thread();
#endif
    }

    void on_scheduler_exit(bool) override {}

private:
    const std::vector<std::uint16_t> &_cores;
    [[maybe_unused]] TBBTree *_tree;
};

class BatchTask : public ::tbb::task
{
public:
    BatchTask(TBBTree *tree, const std::vector<benchmark::WorkloadTuple> &workload_tuples,
              std::atomic<std::size_t> &workload_index, const std::uint16_t batch_size)
        : _tree(tree), _workload_tuples(workload_tuples), _workload_index(workload_index), _batch_size(batch_size)
    {
    }

    virtual ~BatchTask() {}
    virtual ::tbb::task *execute();

private:
    TBBTree *_tree;
    const std::vector<benchmark::WorkloadTuple> &_workload_tuples;
    std::atomic<std::size_t> &_workload_index;
    const std::uint16_t _batch_size;
};
} // namespace indices::blinktree::tbb