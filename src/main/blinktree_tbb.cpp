#include <argparse.hpp>
#include <indices/blinktree/tbb/tbb_benchmark.h>
#include <util/cli_arguments.h>

int main(int count_arguments, const char **arguments)
{
    auto cli_arguments = util::CLIArguments{};

    argparse::ArgumentParser argument_parser(arguments[0]);
    argument_parser.add_argument("cores")
        .help("Range of the number of cores (1 for using 1 core, 1: for using 1 up to available cores, 1:4 for using "
              "cores from 1 to 4).")
        .default_value(std::string("1"));
    argument_parser.add_argument("-b", "--batch-size")
        .help("Number of operations launched at a time (per core).")
        .default_value(std::uint16_t(50))
        .action([](const std::string &value) { return std::uint16_t(std::stoi(value)); });
    argument_parser.add_argument("-s", "--steps")
        .help("Steps, how number of cores is increased (1,2,4,6,.. for -s 2).")
        .default_value(std::uint16_t(2))
        .action([](const std::string &value) { return std::uint16_t(std::stoi(value)); });
    argument_parser.add_argument("-i", "--iterations")
        .help("Number of iterations for each workload")
        .default_value(std::uint16_t(1))
        .action([](const std::string &value) { return std::uint16_t(std::stoi(value)); });
    argument_parser.add_argument("-sco", "--system-core-order")
        .help("Use systems core order. If not, cores are ordered by node id (should be preferred).")
        .implicit_value(true)
        .default_value(false);
    argument_parser.add_argument("-p", "--perf")
        .help("Use performance counter.")
        .implicit_value(true)
        .default_value(false);
    argument_parser.add_argument("--print-stats")
        .help("Print tree statistics after every iteration.")
        .implicit_value(true)
        .default_value(false);
    argument_parser.add_argument("-f", "--workload-files")
        .help("Files containing the workloads (workloads/fill workloads/mixed for example).")
        .nargs(2)
        .default_value(
            std::vector<std::string>{"workloads/fill_randint_workloada", "workloads/mixed_randint_workloada"});
    argument_parser.add_argument("-o", "--out")
        .help("Name of the file to log the results.")
        .default_value(std::string{""});

    // Parse arguments.
    try
    {
        argument_parser.parse_args(count_arguments, arguments);
        cli_arguments = util::CLIArguments::from_argparse(argument_parser);
    }
    catch (std::runtime_error &e)
    {
        std::cout << argument_parser << std::endl;
        return 1;
    }

    indices::blinktree::tbb::TBBBenchmark tbb_benchmark(cli_arguments);
    tbb_benchmark.execute();

    return 0;
}