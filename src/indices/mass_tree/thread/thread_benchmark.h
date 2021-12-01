#pragma once

#include "mtIndexAPI.hh"
#include "query_masstree.hh"
#include <benchmark/abstract_benchmark.h>
#include <benchmark/workload_tuple.h>
#include <config.h>
#include <cstdint>
#include <indices/types.h>
#include <vector>

namespace indices::masstree::thread {
class ThreadBenchmark : public benchmark::AbstractBenchmark
{
public:
    using MassTree = mt_index<Masstree::default_table>;

    explicit ThreadBenchmark(const util::CLIArguments &config);
    virtual ~ThreadBenchmark() = default;

    void execute();

protected:
    virtual void clean_tree()
    {
        _main_thread_info->rcu_quiesce();
        //        _main_thread_info->deallocate_ti();
        delete _tree;
    }

private:
    void execute_iteration();
    void check_and_print_tree();

    MassTree *_tree = nullptr;
    threadinfo *_main_thread_info = nullptr;
};
} // namespace indices::masstree::thread
