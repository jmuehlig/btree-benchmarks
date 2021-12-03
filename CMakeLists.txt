cmake_minimum_required(VERSION 3.10)
project(blinktree_benchmarks)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_FLAGS "-pedantic -Wall -Wextra -march=native -mtune=native -Wno-invalid-offsetof")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -g -DNDEBUG")
set(CMAKE_BUILD_TYPE RELEASE)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Download TBB
include(src/lib/tbb/cmake/TBBBuild.cmake)
include(src/lib/tbb/cmake/TBBGet.cmake)
tbb_get(SAVE_TO src/lib/tbb SOURCE_CODE RELEASE_TAG "v2020.3")
tbb_build(
    TBB_ROOT src/lib/tbb/v2020/oneTBB-2020.3
    USER_DEFINED_ARGS "tbb_build_dir=src/lib/tbb"
    CONFIG_DIR src/lib/tbb/v2020/oneTBB-2020.3
)

include_directories(src src/lib src/lib/tbb/v2020/oneTBB-2020.3/include)
link_directories(tbb_cmake_build/tbb_cmake_build_subdir_release)

if ("$ENV{DIR_LIB}" STREQUAL "")
    set(ENV{DIR_LIB} /usr/lib/x86_64-linux-gnu)
endif()

set(BENCHMARK_SRC
    src/benchmark/abstract_benchmark.cpp
    src/benchmark/workload.cpp
    src/util/core_configuration.cpp
)

set(BLINKTREE_SRC
    src/indices/blinktree/node.cpp
    src/indices/blinktree/tree.cpp
    src/indices/blinktree/consistency_checker.cpp
    src/indices/blinktree/tree_statistics.cpp
)

set(TBB_SRC
    src/main/blinktree_tbb.cpp

    # BLinkTree TBB
    src/indices/blinktree/tbb/tbb_benchmark.cpp
    src/indices/blinktree/tbb/insert_task.cpp
    src/indices/blinktree/tbb/lookup_task.cpp
    src/indices/blinktree/tbb/update_task.cpp
)

add_executable(blinktree_thread_spinlock
    src/main/blinktree_thread.cpp

    # Benchmark
    ${BENCHMARK_SRC}

    # BLinkTree
    ${BLINKTREE_SRC}

    # BLinkTree Thread
    src/indices/blinktree/thread/latched/thread_tree.cpp
    src/indices/blinktree/thread/thread_benchmark.cpp
)

add_executable(blinktree_thread_rwlock
    src/main/blinktree_thread.cpp

    # Benchmark
    ${BENCHMARK_SRC}

    # BLinkTree
    ${BLINKTREE_SRC}

    # BLinkTree Thread
    src/indices/blinktree/thread/latched/thread_tree.cpp
    src/indices/blinktree/thread/thread_benchmark.cpp
)

add_executable(blinktree_thread_olfit
    src/main/blinktree_thread.cpp

    # Benchmark
    ${BENCHMARK_SRC}

    # BLinkTree
    ${BLINKTREE_SRC}

    # BLinkTree Thread
    src/indices/blinktree/epoch_manager.cpp
    src/util/mpsc_queue.cpp
    src/indices/blinktree/thread/optimistic/thread_tree.cpp
    src/indices/blinktree/thread/thread_benchmark.cpp
)

add_executable(blinktree_tbb_lock
    # Benchmark
    ${BENCHMARK_SRC}

    # BLinkTree
    ${BLINKTREE_SRC}

    # BLinkTree TBB
    ${TBB_SRC}
)

add_executable(blinktree_tbb_rw_lock
    # Benchmark
    ${BENCHMARK_SRC}

    # BLinkTree
    ${BLINKTREE_SRC}

    # BLinkTree TBB
    ${TBB_SRC}
)

add_executable(blinktree_tbb_tsx_lock
    # Benchmark
    ${BENCHMARK_SRC}

    # BLinkTree
    ${BLINKTREE_SRC}

    # BLinkTree TBB
    ${TBB_SRC}
)

add_executable(blinktree_tbb_tsx_rw_lock
    # Benchmark
    ${BENCHMARK_SRC}

    # BLinkTree
    ${BLINKTREE_SRC}

    # BLinkTree TBB
    ${TBB_SRC}
)

add_executable(blinktree_tbb_no_lock
    # Benchmark
    ${BENCHMARK_SRC}

    # BLinkTree
    ${BLINKTREE_SRC}

    # BLinkTree TBB
    ${TBB_SRC}
)

add_executable(blinktree_tbb_olfit
    # Benchmark
    ${BENCHMARK_SRC}

    # BLinkTree
    ${BLINKTREE_SRC}

    # BLinkTree TBB
    src/indices/blinktree/epoch_manager.cpp
    src/util/mpsc_queue.cpp
    ${TBB_SRC}
)

add_executable(olc_btree_thread
    src/main/olc_btree_thread.cpp

    # Benchmark
    ${BENCHMARK_SRC}

    # Optimistic BTree
    src/indices/olc_btree/thread/thread_benchmark.cpp
)

add_executable(bwtree_thread
    src/main/bw_tree_thread.cpp

    # Benchmark
    ${BENCHMARK_SRC}

    # BwTree
    src/indices/bw_tree/thread/thread_benchmark.cpp
    src/indices/bw_tree/thread/bwtree.cpp
)

add_executable(mass_tree_thread
    src/main/mass_tree_thread.cpp

    # Benchmark
    ${BENCHMARK_SRC}

    # Mass Tree
    src/indices/mass_tree/thread/thread_benchmark.cpp
    src/indices/mass_tree/thread/mtIndexAPI.cc
    src/indices/mass_tree/thread/misc.cc
    src/indices/mass_tree/thread/checkpoint.cc
    src/indices/mass_tree/thread/query_masstree.cc
    src/indices/mass_tree/thread/value_string.cc
    src/indices/mass_tree/thread/value_array.cc
    src/indices/mass_tree/thread/value_versioned_array.cc
    src/indices/mass_tree/thread/string_slice.cc
    src/indices/mass_tree/thread/testrunner.cc
    src/indices/mass_tree/thread/kvio.cc
    src/indices/mass_tree/thread/json.cc
    src/indices/mass_tree/thread/string.cc
    src/indices/mass_tree/thread/straccum.cc
    src/indices/mass_tree/thread/str.cc
    src/indices/mass_tree/thread/msgpack.cc
    src/indices/mass_tree/thread/clp.cc
    src/indices/mass_tree/thread/kvrandom.cc
    src/indices/mass_tree/thread/compiler.cc
    src/indices/mass_tree/thread/kvthread.cc

)

target_compile_definitions(blinktree_thread_spinlock PUBLIC blinktree_thread_spinlock)
target_compile_definitions(blinktree_thread_rwlock PUBLIC blinktree_thread_rwlock)
target_compile_definitions(blinktree_thread_olfit PUBLIC blinktree_thread_olfit)
target_compile_definitions(blinktree_tbb_rw_lock PUBLIC blinktree_tbb_rw_lock)
target_compile_definitions(blinktree_tbb_lock PUBLIC blinktree_tbb_lock)
target_compile_definitions(blinktree_tbb_tsx_lock PUBLIC blinktree_tbb_tsx_lock)
target_compile_definitions(blinktree_tbb_tsx_rw_lock PUBLIC blinktree_tbb_tsx_rw_lock)
target_compile_definitions(blinktree_tbb_no_lock PUBLIC blinktree_tbb_no_lock)
target_compile_definitions(blinktree_tbb_olfit PUBLIC blinktree_tbb_olfit)
target_compile_definitions(olc_btree_thread PUBLIC olc_btree_thread)
target_compile_definitions(bwtree_thread PUBLIC bwtree_thread BWTREE_NODEBUG ALL_PUBLIC)
target_compile_definitions(mass_tree_thread PUBLIC mass_tree_thread)

target_compile_options(mass_tree_thread PUBLIC -include src/indices/mass_tree/thread/config.h)

target_link_libraries(blinktree_thread_spinlock pthread numa)
target_link_libraries(blinktree_thread_rwlock pthread numa)
target_link_libraries(blinktree_thread_olfit pthread numa)
target_link_libraries(blinktree_tbb_lock pthread numa tbb)
target_link_libraries(blinktree_tbb_rw_lock pthread numa tbb)
target_link_libraries(blinktree_tbb_tsx_lock pthread numa tbb)
target_link_libraries(blinktree_tbb_tsx_rw_lock pthread numa tbb)
target_link_libraries(blinktree_tbb_no_lock pthread numa tbb)
target_link_libraries(blinktree_tbb_olfit pthread numa tbb)
target_link_libraries(olc_btree_thread pthread numa)
target_link_libraries(bwtree_thread pthread numa atomic)
target_link_libraries(mass_tree_thread pthread numa $ENV{DIR_LIB}/libtcmalloc_minimal.so.4)

add_custom_target(ycsb-a ./scripts/generate_ycsb a randint)
add_custom_target(ycsb-c ./scripts/generate_ycsb c randint)
