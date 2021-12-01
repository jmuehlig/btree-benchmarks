#include "thread_benchmark.h"
#include <atomic>
#include <config.h>
#include <hardware/thread.h>
#include <thread>

using namespace indices::bwtree::thread;
using namespace benchmark;

ThreadBenchmark::ThreadBenchmark(const util::CLIArguments &config) : AbstractBenchmark(config), _tree(nullptr)
{
}

void ThreadBenchmark::execute()
{
    do
    {
        if (this->is_fill_phase())
        {
            this->_tree = new BwTree();
        }

        // Running the iteration.
        this->start_timer();
        this->execute_iteration();
        this->stop_timer();

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
    this->_tree->UpdateThreadLocal(cores.size());
    for (auto core : cores)
    {
        tree_threads[thread_index] = std::thread([this, &workload_pointer, thread_index] {
            this->_tree->AssignGCID(thread_index);
            auto count_items = this->workload().count_current_phase();
            const auto &items = this->workload().current_workload();

            std::size_t current_pointer;

            while ((current_pointer = workload_pointer.fetch_add(this->config().batch_size())) < count_items)
            {
                const auto items_to_execute =
                    std::min(std::size_t(this->config().batch_size()), count_items - current_pointer);
                for (auto i = 0u; i < items_to_execute; i++)
                {
                    const auto &tuple = items[current_pointer + i];
                    if (tuple == WorkloadTuple::Insert || tuple == WorkloadTuple::Update)
                    {
                        this->_tree->Insert(tuple.key(), tuple.value());
                    }
                    else
                    {
                        auto value = this->_tree->GetValue(tuple.key());
                        ThreadBenchmark::DoNotOptimize(value);
                    }
                }
            }

            this->_tree->UnregisterThread(thread_index);
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
}