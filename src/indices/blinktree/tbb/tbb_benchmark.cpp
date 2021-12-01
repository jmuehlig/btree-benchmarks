#include "tbb_benchmark.h"
#include "insert_task.h"
#include "lookup_task.h"
#include "update_task.h"
#include <config.h>
#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>
#include <tbb/task_scheduler_init.h>

using namespace indices::blinktree::tbb;

TBBBenchmark::TBBBenchmark(const util::CLIArguments &config) : AbstractBenchmark(config)
{
}

void TBBBenchmark::execute()
{
    do
    {
        const auto count_threads = this->core_config().current().physical_cores().size();
        if (this->is_fill_phase())
        {
#ifdef blinktree_tbb_olfit
            this->_tree = new TBBTree(count_threads);
#else
            this->_tree = new TBBTree();
#endif
        }
#ifdef blinktree_tbb_olfit
        this->_tree->start_epoch();
#endif
        ::tbb::task_scheduler_init scheduler(count_threads, 1024 * 1024 * 16);
        SchedulerObserver scheduler_observer(this->core_config().current().physical_cores(), this->_tree);
        scheduler_observer.observe(true);

        const auto &workload_tuples = this->workload().current_workload();
        std::atomic<std::size_t> workload_index{0};

        this->start_timer();
        ::tbb::parallel_for(0ul, count_threads, [&](auto) {
            auto root_batch_task = new (::tbb::task::allocate_root())
                BatchTask(this->_tree, workload_tuples, workload_index, this->config().batch_size());
            ::tbb::task::spawn_root_and_wait(*root_batch_task);
        });
        this->stop_timer();
#ifdef blinktree_tbb_olfit
        this->_tree->stop_epoch();
#endif

        scheduler.terminate();

        this->print_current_timer();
        this->check_and_print_tree();

    } while (is_finished() == false);
}

::tbb::task *BatchTask::execute()
{
    const auto count_items = this->_workload_tuples.size();
    const std::size_t workload_index = this->_workload_index.fetch_add(this->_batch_size);

    if (workload_index >= count_items)
    {
        return nullptr;
    }

    const auto batch_size = std::min(std::size_t(this->_batch_size), count_items - workload_index);

    this->add_ref_count(batch_size + 1);
    for (std::size_t item_index = 0; item_index < batch_size; item_index++)
    {
        const auto &tuple = this->_workload_tuples[workload_index + item_index];
        if (tuple == benchmark::WorkloadTuple::Insert)
        {
            this->spawn(*new (this->allocate_child()) InsertTask(this->_tree, tuple.key(), tuple.value()));
        }
        else if (tuple == benchmark::WorkloadTuple::Update)
        {
            this->spawn(*new (this->allocate_child()) UpdateTask(this->_tree, tuple.key(), tuple.value()));
        }
        else
        {
            this->spawn(*new (this->allocate_child()) LookupTask(this->_tree, tuple.key()));
        }
    }

    this->wait_for_all();
    this->recycle_as_continuation();
    return this;
}

void TBBBenchmark::check_and_print_tree()
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