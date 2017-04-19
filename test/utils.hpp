#include <ostream>

#include <deepstream/core/buffer.hpp>

namespace deepstream {

std::ostream &operator<<(std::ostream &os, WSState state)
{
    const char *states[] = {
        "ERROR",
        "OPEN",
        "CLOSED"
    };
    os << states[static_cast<int>(state)];
    return os;
}

std::ostream &operator<<(std::ostream &os, const Buffer &buff)
{
    os << std::string(buff.data(), buff.size());
    return os;
}
}
