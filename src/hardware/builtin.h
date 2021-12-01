#pragma once
#include "system.h"
#include <cstdint>

namespace hardware {
class Builtin
{
public:
    inline static void pause()
    {
#if defined(__x86_64__) || defined(__amd64__)
        __builtin_ia32_pause();
#elif defined(__arm__)
        // TODO: yield
#endif
    }
};
} // namespace hardware