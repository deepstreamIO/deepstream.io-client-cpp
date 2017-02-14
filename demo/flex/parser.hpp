#ifndef DEEPSTREAM_PARSER_HPP
#define DEEPSTREAM_PARSER_HPP

#include <cstddef>

namespace deepstream {
namespace parser {
    struct Location {
        explicit Location(std::size_t offset, std::size_t length)
            : offset_(offset)
            , length_(length)
        {
        }

        std::size_t offset_;
        std::size_t length_;
    };

    typedef deepstream_parser_state State;
}
}

struct deepstream_parser_state {
    explicit deepstream_parser_state()
        : tokenizing_header(true)
        , offset(0)
    {
    }

    bool tokenizing_header;
    std::size_t offset;
};

#endif
