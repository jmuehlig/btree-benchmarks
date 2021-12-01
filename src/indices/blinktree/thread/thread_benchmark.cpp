#include "thread_benchmark.h"
#include <atomic>
#include <config.h>
#include <hardware/thread.h>
#include <iostream>
#include <thread>

using namespace indices::blinktree::thread;

ThreadBenchmark::ThreadBenchmark(const util::CLIArguments &config) : AbstractBenchmark(config)
{
}

void ThreadBenchmark::execute()
{
    do
    {
        if (this->is_fill_phase())
        {
#ifdef blinktree_thread_olfit
            const auto count_threads = this->core_config().current().physical_cores().size();
            this->_tree = new ThreadTree(count_threads);
#else
            this->_tree = new ThreadTree();
#endif
        }

        // Running the iteration.
#ifdef blinktree_thread_olfit
        this->_tree->start_epoch();
#endif
        this->start_timer();
        this->execute_iteration();
        this->stop_timer();
#ifdef blinktree_thread_olfit
        this->_tree->stop_epoch();
#endif

        // Print result.
        this->print_current_timer();

        // Check and print tree.
        this->check_and_print_tree();
    } while (this->is_finished() == false);
}

void ThreadBenchmark::execute_iteration()
{
    std::atomic<std::size_t> workload_pointer;
    workload_pointer.store(0);

    const auto &cores = this->core_config().current().physical_cores();
    std::vector<std::thread> tree_threads(cores.size());
    std::size_t thread_index = 0;
    for (auto core : cores)
    {
        tree_threads[thread_index] = std::thread([this, &workload_pointer] {
#ifdef blinktree_thread_olfit
            this->_tree->register_thread();
#endif
            auto count_items = this->workload().count_current_phase();
            const auto &items = this->workload().current_workload();

            std::size_t current_pointer = 0;

            while ((current_pointer = workload_pointer.fetch_add(this->config().batch_size())) < count_items)
            {
                const auto items_to_execute =
                    std::min(std::size_t(this->config().batch_size()), count_items - current_pointer);
                for (auto i = 0u; i < items_to_execute; i++)
                {
                    const auto &tuple = items[current_pointer + i];
                    if (tuple == benchmark::WorkloadTuple::Insert)
                    {
                        this->_tree->insert(tuple.key(), tuple.value());
                    }
                    else if (tuple == benchmark::WorkloadTuple::Update)
                    {
                        this->_tree->update(tuple.key(), tuple.value());
                    }
                    else
                    {
                        const auto value = this->_tree->find(tuple.key());
                        ThreadBenchmark::DoNotOptimize(value);
                    }
                }
            }
        });

        hardware::Thread::pin(tree_threads[thread_index], core);

        thread_index++;
    }

    for (auto &tree_thread : tree_threads)
    {
        tree_thread.join();
    }
}

void ThreadBenchmark::check_and_print_tree()
{
    if (this->config().is_check_tree())
    {
        this->_tree->check();
    }

    if (this->config().is_print_tree_statistics())
    {
        this->_tree->print_statistics();
    }
}
