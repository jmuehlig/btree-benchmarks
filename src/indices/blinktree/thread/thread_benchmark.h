#pragma once

#include <benchmark/abstract_benchmark.h>
#include <cstdint>
#include <vector>

#ifdef blinktree_thread_olfit
#include "indices/blinktree/thread/optimistic/thread_tree.h"
using ThreadTree = indices::blinktree::thread::optimistic::ThreadTree;
#else
#include "indices/blinktree/thread/latched/thread_tree.h"
using ThreadTree = indices::blinktree::thread::latched::ThreadTree;
#endif

namespace indices::blinktree::thread {
class ThreadBenchmark : public benchmark::AbstractBenchmark
{
public:
    explicit ThreadBenchmark(const util::CLIArguments &config);
    virtual ~ThreadBenchmark() = default;

    void execute();

protected:
    virtual void clean_tree() { delete _tree; }

private:
    void execute_iteration();
    void check_and_print_tree();

    ThreadTree *_tree = nullptr;
};
} // namespace indices::blinktree::thread