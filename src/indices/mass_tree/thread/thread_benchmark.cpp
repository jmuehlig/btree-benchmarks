#include "thread_benchmark.h"
#include <atomic>
#include <config.h>
#include <hardware/thread.h>
#include <thread>

using namespace indices::masstree::thread;
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
            this->_tree = new MassTree();
            this->_main_thread_info = threadinfo::make(threadinfo::TI_MAIN, -1);
            this->_tree->setup(this->_main_thread_info);
            //            this->_main_thread_info->rcu_start();
        }

        // Running the iteration.
        this->start_timer();
        this->execute_iteration();
        this->stop_timer();
        //        if (this->is_fill_phase() == false)
        //        {
        //            this->_main_thread_info->rcu_stop();
        //        }

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
        tree_threads[thread_index] = std::thread([this, &workload_pointer, thread_index] {
            auto *thread_info = threadinfo::make(threadinfo::TI_MAIN, thread_index);
            auto count_items = this->workload().count_current_phase();
            const auto &items = this->workload().current_workload();

            std::size_t current_pointer;
            auto garbage_collection_counter = 0u;

            while ((current_pointer = workload_pointer.fetch_add(this->config().batch_size())) < count_items)
            {
                const auto items_to_execute =
                    std::min(std::size_t(this->config().batch_size()), count_items - current_pointer);
                for (auto i = 0u; i < items_to_execute; i++)
                {
                    const auto &tuple = items[current_pointer + i];
                    const auto key = __bswap_64(tuple.key());
                    if (tuple == WorkloadTuple::Insert || tuple == WorkloadTuple::Update)
                    {
                        this->_tree->put(reinterpret_cast<const char *>(&key), sizeof(key_type),
                                         reinterpret_cast<const char *>(tuple.value_pointer()), sizeof(value_type),
                                         thread_info);
                    }
                    else
                    {
                        Str value;
                        this->_tree->get(reinterpret_cast<const char *>(&key), sizeof(key_type), value, thread_info);
                        ThreadBenchmark::DoNotOptimize(*reinterpret_cast<const value_type *>(value.s));
                    }

                    if ((garbage_collection_counter++) % 4096 == 0u)
                    {
                        thread_info->rcu_quiesce();
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

    this->_main_thread_info->rcu_quiesce();
}

void ThreadBenchmark::check_and_print_tree()
{
}