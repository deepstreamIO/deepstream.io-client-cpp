/*
 * this file requires a quite recent Bison version because of the api.pure
 * define
 */
%require "3.0.4"
%locations
%define api.pure full
%define parse.error verbose
%param{struct state* p_state}


%initial-action
{
    @$.first_line = -1;
    @$.first_column = -1;
    @$.last_line = -1;
    @$.last_column = -1;
}



%{
#include "parser.h"
%}

%token MPS 31
%token MS 30
%token NON_SEPARATOR

%token ACK

%token TOPIC_EVENT
%token TOPIC_RECORD
%token TOPIC_UNKNOWN

%token ACTION_SUBSCRIBE
%token ACTION_UNSUBSCRIBE
%token ACTION_LISTEN
%token ACTION_UNLISTEN
%token ACTION_CREATE_READ

%%
message: topic_event  MPS event_actions mps_f payload MS
       | topic_event  MPS ack MPS event_actions mps_f payload MS
       | topic_record MPS record_actions mps_f payload MS;
       ;

event_actions: ACTION_SUBSCRIBE
             | ACTION_UNSUBSCRIBE
             | ACTION_LISTEN
             | ACTION_UNLISTEN

record_actions: ACTION_SUBSCRIBE
              | ACTION_UNSUBSCRIBE
              | ACTION_LISTEN
              | ACTION_UNLISTEN
              | ACTION_CREATE_READ

topic_event: TOPIC_EVENT { printf("event\n"); };
topic_record: TOPIC_RECORD { printf("record\n"); };


ack: ACK {printf("ack\n"); };

mps_f: MPS { p_state->tokenize_header = false; };

payload: NON_SEPARATOR payload_list { ++p_state->num_data; };

payload_list:
			| MPS NON_SEPARATOR payload_list { ++p_state->num_data; }
			;
