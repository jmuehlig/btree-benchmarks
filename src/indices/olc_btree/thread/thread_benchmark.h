#pragma once

#include "btree_olc.h"
#include <benchmark/abstract_benchmark.h>
#include <benchmark/workload_tuple.h>
#include <config.h>
#include <cstdint>
#include <indices/types.h>
#include <vector>

namespace indices::optimistic_btree::thread {
class ThreadBenchmark : public benchmark::AbstractBenchmark
{
public:
    using OLCBTree = btreeolc::BTree<indices::key_type, indices::value_type>;

    explicit ThreadBenchmark(const util::CLIArguments &config);
    virtual ~ThreadBenchmark() = default;

    void execute();

protected:
    virtual void clean_tree() { delete _tree; }

private:
    void execute_iteration();
    void check_and_print_tree();

    OLCBTree *_tree = nullptr;
};
} // namespace indices::optimistic_btree::thread
