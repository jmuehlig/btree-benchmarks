#include "workload.h"
#include <algorithm>
#include <iostream>

using namespace benchmark;

void Workload::load(const char *fill_file_name, const char *workload_file_name)
{
    {
        std::ifstream fill_file(fill_file_name);
        std::size_t lookup_counter{0u}, update_counter{0u};
        this->parse_file(fill_file, this->_fill_phase_tuples, this->_fill_inserts, lookup_counter, update_counter);
    }

    {
        std::ifstream workload_file(workload_file_name);
        this->parse_file(workload_file, this->_workload_phase_tuples, this->_inserts, this->_lookups, this->_updates);
    }
}

void Workload::parse_file(std::ifstream &in_stream, std::vector<WorkloadTuple> &tuples, std::size_t &insert_counter,
                          std::size_t &lookup_counter, std::size_t &update_counter)
{
    std::srand(1337);
    std::string operation_name;
    key_type key;

    while (in_stream >> operation_name >> key)
    {
        if (operation_name == "INSERT")
        {
            tuples.push_back({WorkloadTuple::Insert, key, value_type(std::rand())});
            insert_counter++;
        }
        else if (operation_name == "READ")
        {
            tuples.push_back({WorkloadTuple::Lookup, key});
            lookup_counter++;
        }
        else if (operation_name == "UPDATE")
        {
            tuples.push_back({WorkloadTuple::Update, key, value_type(std::rand())});
            update_counter++;
        }
    }
}

std::ostream &benchmark::operator<<(std::ostream &stream, const Workload &workload)
{
    stream << " fill: " << workload.count_fill_inserts();

    if (workload.has_benchmark())
    {
        stream << " / mixed:";
        stream << " insert(" << workload.count_benchmark_inserts() << ")";
        stream << " update(" << workload.count_benchmark_updates() << ")";
        stream << " read(" << workload.count_benchmark_lookups() << ")";
    }

    return stream;
}