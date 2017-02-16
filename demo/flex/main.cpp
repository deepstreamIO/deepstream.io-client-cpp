#include <cstdio>
#include <cstring>

#include <cstddef>

extern "C" {
#include "lex.yy.h"
}
#include "parser.hpp"

#include <cassert>

// control memory consumption with yyalloc, yyrealloc, yyfree
int main()
{
    // char string[] = "E|A|S+";
    // char string[] = "E|A|S|foo|bar|baz+";
    // char string[] = "E|L|name+";
    char string[] = "E|A|S|foo|bar|baz+";
    std::size_t size = strlen(string);

    for (std::size_t i = 0; i < size; ++i) {
        if (string[i] == '+')
            string[i] = '\x1e';
        if (string[i] == '|')
            string[i] = '\x1f';
    }

    yyscan_t scanner;
    int ret = yylex_init(&scanner);
    if (ret != 0) {
        std::perror("yylex_init");
        return 1;
    }

    char* const input = new char[size + 2];
    std::memset(input, 0, size);
    std::strncpy(input, string, size);

    for (unsigned iter = 0; iter < 1u << 20; ++iter) {
        deepstream::parser::State parser_state;
        yyset_extra(&parser_state, scanner);

        // yy_scan_bytes() copies the string!
        // consider fmemopen() as an alternative
        YY_BUFFER_STATE buffer = yy_scan_buffer(input, size + 2, scanner);
        yy_switch_to_buffer(buffer, scanner);

        for (ret = 0; ret > EOF; ret = yylex(scanner)) {
        }
        // std::printf("main after yylex()\n");

        if (ret < EOF) {
            std::perror("yylex");
            return 1;
        }

        yy_delete_buffer(buffer, scanner);
    }

    yylex_destroy(scanner);
}
