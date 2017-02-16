#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include <stdio.h>

enum Topic { UNKNOWN_TOPIC,
    EVENT,
    RECORD };

enum Action {
    UNKNOWN_ACTION,
    SUBSCRIBE,
    UNSUBSCRIBE,
    LISTEN,
    UNLISTEN,
    CREATE_READ
};

struct message {
    enum Topic topic;
    bool isAck;
    enum Action action;
};

struct state {
    const char* const first;
    const char* const last;
    bool tokenize_header;
    const char* p;
    size_t num_data;
};

struct YYLTYPE;

int yylex(void*, struct YYLTYPE*, struct state*);
void yyerror(const struct YYLTYPE*, const struct state*, const char*);

#endif
