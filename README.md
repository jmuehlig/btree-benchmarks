# BLinkTree Micro Benchmarks
**This repository is available for reproducibility of our paper [MxTasks: How to Make Efficient Synchronization and Prefetching Easy](https://dl.acm.org/doi/pdf/10.1145/3448016.3457268)**.

This is a set of micro benchmarks for a BLinkTree using different programming models like Threads and Intel Threading Building Blocks running on top of linux.

## Dependencies
**Required:**
* `cmake` in version `3.10` or higher.
* `clang` in version `10` or higher (for supporting `c++` with standard `17`)
* `libnuma` (or `libnuma-dev`)
* `tcmalloc` (for running the Masstree)

**Optional:**
* `perf` for recording performance counters (Please note: Installing perf takes some other packages)

## How to build
* Use `cmake .` to generate a `Makefile`
* Use `make` to build the project. 

## Binaries
`make` will create the following executables:
* `bin/blinktree_thread_spinlock`: Benchmark for a `p_thread`-based parallel B link tree using spinlocks to protect nodes.
* `bin/blinktree_thread_rw_lock`: Benchmark for a `p_thread`-based parallel B link tree using reader-writer-locks to protect nodes, allowing parallel reads but synchronizing writes.
* `bin/blinktree_thread_olfit`: Benchmark for a `p_thread`-based parallel B link tree using versioning to allow parallel reads without locks.
* `bin/blinktree_tbb_lock`: Benchmark for a task-based parallel B link tree using [Intel TBB](https://github.com/intel/tbb). Nodes are protected by `queuing_mutex`.
* `bin/blinktree_tbb_rw_lock`: Benchmark for a task-based parallel B link tree using [Intel TBB](https://github.com/intel/tbb). Nodes are protected by `queuing_rw_mutex`, allowing parallel reading tasks on a node.
* `bin/blinktree_tbb_tsx_lock`: Benchmark for a task-based parallel B link tree using [Intel TBB](https://github.com/intel/tbb). Nodes are protected by `speculative_spin_rw_mutex` which uses transactional memory to acquire the lock. Nodes can be read parallel.
* `bin/blinktree_tbb_no_lock`: Benchmark for a task-based parallel B link tree using [Intel TBB](https://github.com/intel/tbb). Nodes are not protected in the second benchmark phase.
* `bin/blinktree_tbb_olfit`: Benchmark for a task-based parallel B link tree using [Intel TBB](https://github.com/intel/tbb) and versioning to allow parallel reads without locks.
* `bin/olc_btree_thread`: Benchmark for the `p_threads`-based B tree using optimistic lock coupling ([see on Github](https://github.com/wangziqi2016/index-microbench/tree/master/BTreeOLC), [read the paper](http://sites.computer.org/debull/A19mar/p73.pdf)).
* `bin/bwtree_thread`: Benchmark for the open BwTree ([see on Github](https://github.com/wangziqi2013/BwTree), [read the paper](https://dl.acm.org/doi/pdf/10.1145/3183713.3196895)).
* `bin/mass_tree_thread`: Benchmark for the Masstree ([see on Github](https://github.com/kohler/masstree-beta), [read the paper](https://dl.acm.org/doi/pdf/10.1145/2168836.2168855)).

## Workload
We support the [Yahoo! Cloud System Benchmark (YCSB)](https://github.com/brianfrankcooper/YCSB).
The target `make ycsb-a` will generate `workloada` with random keys, equivalent for `make ycsb-c`.
The workloads can be specified in `workload_specification/`
    
## How to run
Use one of the binaries with `-h` for help output.
Example for `./bin/blinktree_thread_spinlock -h`:

    Usage: ./bin/blinktree_thread_spinlock [options] cores

    Positional arguments:
    cores                    	Range of the number of cores (1 for using 1 core, 1: for using 1 up to available cores, 1:4 for using cores from 1 to 4).
    
    Optional arguments:
    -h --help                	show this help message and exit
    -b --batch-size          	Number of operations launched at a time (per core).
    -s --steps               	Steps, how number of cores is increased (1,2,4,6,.. for -s 2).
    -i --iterations          	Number of iterations for each workload
    -sco --system-core-order 	Use systems core order. If not, cores are ordered by node id (should be preferred).
    -p --perf                	Use performance counter.
    --print-stats            	Print tree statistics after every iteration.
    -f --workload-files      	Files containing the workloads (workloads/fill workloads/mixed for example).
    -o --out                 	Name of the file to log the results.
    
    Examples: 
      ./bin/blinktree_thread_spinlock 1:32 -s 2   for using increasing core count 1,2,4,6,..,32
      ./bin/blinktree_thread_spinlock 1           for using just a single core

## Understanding the output
Running one of the executables will generate output to the console:
    
    $ ./bin/blinktree_thread_spinlock 1:4
    core configuration:
    1: 0
    2: 0 1
    4: 0 1 2 3
    workload:  fill: 10000000 / mixed: insert(0) update(0) read(10000000)
    1 1 0 	4,826 ms 	2.07211e+06 op/s
    1 1 1 	4,491 ms 	2.22668e+06 op/s
    2 1 0 	2,849 ms 	3.51e+06 op/s
    2 1 1 	2,534 ms 	3.94633e+06 op/s
    4 1 0 	1,792 ms 	5.58036e+06 op/s
    4 1 1 	1,520 ms 	6.57895e+06 op/s
    
The first lines will give some statistics about the used cores, loaded workload, and configuration.
The result lines (beginning at line `9`) are specified like follows:
* The first column is the core count (starting with one core, up to four cores).
* The second column is an index of the run within the current core configuration (here: using one run for each phase and each core configuration).
* The third column is an identifier of the phase, where `0` means the fill phase and `1` the workload phase. In this example, the fill phase will contain fifty million inserts and the workload phase will contain the same number of reads.
* The fourth column is the time for this run in milliseconds.
* The fifth column is the throughput, calculated by the time and given workload.
    
## Included Libraries
* **BTreeOLC**: Optimistic BTree implementation for threads, available on [Github](https://github.com/wangziqi2016/index-microbench/tree/master/BTreeOLC)
* **open BwTree**: General purpose, concurrent and lock-free B+-Tree index, available on [Github](https://github.com/wangziqi2013/BwTree).
* **Masstree**: A fast, multi-core key-value store, available on [Github](https://github.com/kohler/masstree-beta).
* **RWSpinLock**: `util/RWSpinLock` is based on the implementation of Facebook's `folly` library, available on [GitHub](https://github.com/facebook/folly/blob/master/folly/synchronization/RWSpinLock.h).
* **argparse**: An argument parsing library, available on [view on github](https://github.com/p-ranav/argparse).