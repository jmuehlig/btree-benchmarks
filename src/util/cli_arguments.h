#pragma once

#include "core_configuration.h"
#include <argparse.hpp>
#include <cstdint>
#include <hardware/system.h>
#include <regex>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace util {
class CLIArguments
{
public:
    CLIArguments() = default;

    CLIArguments(const std::uint16_t min_cores, const std::uint16_t max_cores, const std::uint16_t core_steps,
                 const bool is_logical_cores, const std::uint16_t iterations, const std::uint16_t batch_size,
                 std::vector<std::string> &&workload_files, const std::string &out_file_name, const bool is_check_tree,
                 const bool is_print_tree_statistics, const bool is_use_perf)
        : _min_cores(min_cores), _max_cores(max_cores), _core_steps(core_steps), _is_logical_cores(is_logical_cores),
          _iterations(iterations), _batch_size(batch_size), _workload_files(std::move(workload_files)),
          _out_file_name(out_file_name), _is_check_tree(is_check_tree),
          _is_print_tree_statistics(is_print_tree_statistics), _is_use_perf(is_use_perf)
    {
    }

    CLIArguments(const std::uint16_t min_cores, const std::uint16_t max_cores, const CLIArguments &other)
        : _min_cores(min_cores), _max_cores(max_cores), _core_steps(other.core_steps()),
          _is_logical_cores(other._is_logical_cores), _iterations(other.iterations()), _batch_size(other.batch_size()),
          _workload_files(other._workload_files), _out_file_name(other._out_file_name),
          _is_check_tree(other.is_check_tree()), _is_print_tree_statistics(other.is_print_tree_statistics()),
          _is_use_perf(other.is_use_perf())
    {
    }

    ~CLIArguments() = default;

    CLIArguments &operator=(const CLIArguments &) = default;
    CLIArguments &operator=(CLIArguments &&) = default;

    static CLIArguments from_argparse(argparse::ArgumentParser &argument_parser)
    {
        const auto cores = parse_cores(argument_parser.get<std::string>("cores"));
        return {std::get<0>(cores),
                std::get<1>(cores),
                argument_parser.get<std::uint16_t>("-s"),
                argument_parser.get<bool>("-sco"),
                argument_parser.get<std::uint16_t>("-i"),
                argument_parser.get<std::uint16_t>("-b"),
                argument_parser.get<std::vector<std::string>>("-f"),
                argument_parser.get<std::string>("-o"),
                false,
                argument_parser.get<bool>("--print-stats"),
                argument_parser.get<bool>("-p")};
    }

    inline std::uint16_t min_cores() const { return _min_cores; }
    inline std::uint16_t max_cores() const { return _max_cores; }
    inline std::uint16_t core_steps() const { return _core_steps; }
    inline CoreConfiguration::Type core_configuration_type() const
    {
        return _is_logical_cores ? CoreConfiguration::Logical : CoreConfiguration::NumaAscending;
    }

    inline std::uint16_t iterations() const { return _iterations; }
    inline std::uint16_t batch_size() const { return _batch_size; }
    inline const std::vector<std::string> &workload_files() const { return _workload_files; }

    inline const std::string &out_file_name() const { return _out_file_name; }

    inline bool is_check_tree() const { return _is_check_tree; }
    inline bool is_print_tree_statistics() const { return _is_print_tree_statistics; }

    inline bool is_use_perf() const { return _is_use_perf; }

private:
    std::uint16_t _min_cores = 1u;
    std::uint16_t _max_cores = 1u;
    std::uint16_t _core_steps = 1u;
    bool _is_logical_cores = false;
    std::uint16_t _iterations = 1u;
    std::uint16_t _batch_size = 1u;
    std::vector<std::string> _workload_files;
    std::string _out_file_name = "";
    bool _is_check_tree = false;
    bool _is_print_tree_statistics = false;
    bool _is_use_perf = false;

    static std::pair<std::uint16_t, std::uint16_t> parse_cores(const std::string &cores)
    {
        const std::regex single_core_regex("(\\d+)$");
        const std::regex from_core_regex("(\\d+):$");
        const std::regex core_range_regex("(\\d+):(\\d+)");

        std::stringstream stream(cores);
        std::string token;
        while (std::getline(stream, token, ';'))
        {
            std::smatch match;

            if (std::regex_match(token, match, single_core_regex))
            {
                const auto core = std::stoi(match[1].str());
                return std::make_pair(core, core);
            }
            else if (std::regex_match(token, match, from_core_regex))
            {
                return std::make_pair(std::stoi(match[1].str()), hardware::System::count_cores());
            }
            else if (std::regex_match(token, match, core_range_regex))
            {
                return std::make_pair(std::stoi(match[1].str()), std::stoi(match[2].str()));
            }
        }

        return std::make_pair(0u, 0u);
    }
};
} // namespace util