#pragma once
#include <asm/unistd.h>
#include <cstring>
#include <linux/perf_event.h>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>
#include <unordered_map>

/*
 * For more Performance Counter take a look into the Manual from Intel:
 *  https://software.intel.com/sites/default/files/managed/8b/6e/335279_performance_monitoring_events_guide.pdf
 */

namespace util {
class PerfCounter
{
public:
    PerfCounter(const std::string name, const std::uint64_t type, const std::uint64_t event_id) : _name(name)
    {
        memset(&_perf_event_attribute, 0, sizeof(perf_event_attr));
        _perf_event_attribute.type = type;
        _perf_event_attribute.size = sizeof(perf_event_attr);
        _perf_event_attribute.config = event_id;
        _perf_event_attribute.disabled = true;
        _perf_event_attribute.inherit = 1;
        _perf_event_attribute.inherit_stat = 0;
        _perf_event_attribute.exclude_kernel = false;
        _perf_event_attribute.exclude_hv = false;
        _perf_event_attribute.read_format = PERF_FORMAT_TOTAL_TIME_ENABLED | PERF_FORMAT_TOTAL_TIME_RUNNING;
    }

    PerfCounter(const std::string name, const std::uint32_t event, const std::uint32_t umask, const std::uint32_t cmask)
        : PerfCounter(name, PERF_TYPE_RAW, PerfCounter::event_id_from_mask(event, umask, cmask))
    {
    }

    ~PerfCounter() = default;

    static std::uint32_t event_id_from_mask(const std::uint32_t event, const std::uint32_t umask,
                                            const std::uint32_t cmask)
    {
        return event | (umask << 8) | (std::uint32_t(0) << 18) | (std::uint32_t(0) << 23) | (cmask << 24);
    }

    bool open()
    {
        _file_descriptor = syscall(__NR_perf_event_open, &_perf_event_attribute, 0, -1, -1, 0);
        return _file_descriptor >= 0;
    }

    bool start()
    {
        ioctl(_file_descriptor, PERF_EVENT_IOC_RESET, 0);
        ioctl(_file_descriptor, PERF_EVENT_IOC_ENABLE, 0);
        return ::read(_file_descriptor, &_prev, sizeof(read_format)) == sizeof(read_format);
    }

    bool stop()
    {
        const auto read = ::read(_file_descriptor, &_data, sizeof(read_format)) == sizeof(read_format);
        ioctl(_file_descriptor, PERF_EVENT_IOC_DISABLE, 0);
        return read;
    }

    double read()
    {
        const auto multiplexing_correction =
            static_cast<double>(_data.time_enabled - _prev.time_enabled) / (_data.time_running - _prev.time_running);
        return (_data.value - _prev.value) * multiplexing_correction;
    }

    inline const std::string &name() const { return _name; }
    operator const std::string &() const { return name(); }

    bool operator==(const std::string name) const { return _name == name; }

private:
    struct read_format
    {
        std::uint64_t value = 0;
        std::uint64_t time_enabled = 0;
        std::uint64_t time_running = 0;
    };

    const std::string _name;
    std::int32_t _file_descriptor = -1;
    perf_event_attr _perf_event_attribute;
    read_format _prev;
    read_format _data;
};

class Perf
{
public:
    inline static PerfCounter INSTRUCTIONS = {"instructions", PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS};
    inline static PerfCounter CYCLES = {"cycles", PERF_TYPE_HARDWARE, PERF_COUNT_HW_CPU_CYCLES};
    inline static PerfCounter L1_MISSES = {"l1-misses", PERF_TYPE_HW_CACHE,
                                           PERF_COUNT_HW_CACHE_L1D | (PERF_COUNT_HW_CACHE_OP_READ << 8) |
                                               (PERF_COUNT_HW_CACHE_RESULT_MISS << 16)};
    inline static PerfCounter LLC_MISSES = {"llc-misses", PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_MISSES};
    inline static PerfCounter STALLED_CYCLES_BACKEND = {"stalled-cycles-backend", PERF_TYPE_HARDWARE,
                                                        PERF_COUNT_HW_STALLED_CYCLES_BACKEND};
    inline static PerfCounter STALLS_MEM_ANY = {"stalls-memory-any", 0xA3, 0x14, 20};

    inline bool add(PerfCounter counter)
    {
        if (counter.open())
        {
            _counter.push_back(counter);
            return true;
        }

        return false;
    }

    inline bool add(const std::string name, const std::uint64_t type, const std::uint64_t event_id)
    {
        return add({name, type, event_id});
    }

    inline void start()
    {
        for (auto &counter : _counter)
        {
            counter.start();
        }
    }

    inline void stop()
    {
        for (auto &counter : _counter)
        {
            counter.stop();
        }
    }

    double operator[](const std::string &name)
    {
        for (auto &counter : _counter)
        {
            if (counter == name)
            {
                return counter.read();
            }
        }

        return 0.0;
    }

private:
    std::vector<PerfCounter> _counter;
};
} // namespace util