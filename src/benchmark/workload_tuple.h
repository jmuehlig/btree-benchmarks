#pragma once
#include <indices/types.h>
#include <ostream>

using namespace indices;

namespace benchmark {
class __attribute__((packed)) alignas(32) WorkloadTuple
{
public:
    enum Operation
    {
        Insert,
        Lookup,
        Update
    };

    WorkloadTuple(const Operation operation = Lookup, const key_type key = 0, const value_type value = 0) noexcept
        : _operation(operation), _key(key), _value(value)
    {
    }

    WorkloadTuple(WorkloadTuple const &other) = default;

    bool operator<(const WorkloadTuple &other) const { return _key < other.key(); }
    bool operator>(const WorkloadTuple &other) const { return _key > other.key(); }
    bool operator==(const WorkloadTuple &other) const { return _key == other.key(); }
    bool operator==(const Operation &operation) const { return _operation == operation; }

    inline Operation operation() const { return _operation; }
    inline key_type key() const { return _key; }
    inline const key_type *key_pointer() const { return &_key; }
    inline value_type value() const { return _value; }
    inline value_type *value_pointer() { return &_value; }
    inline const value_type *value_pointer() const { return &_value; }

    friend std::ostream &operator<<(std::ostream &stream, const WorkloadTuple &tuple)
    {
        if (tuple == Insert)
        {
            return stream << "insert(" << tuple.key() << "," << tuple.value() << ")";
        }
        else
        {
            return stream << "lookup(" << tuple.key() << ")";
        }
    }

private:
    Operation _operation;
    key_type _key;
    value_type _value;
};
} // namespace benchmark