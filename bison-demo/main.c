#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "parser.h"
#include "parser.tab.h"

#include <assert.h>


int yylex(void* lvalp, YYLTYPE* p_loc, struct state* p_state)
{
	(void) lvalp;

	assert( p_state );

	const char* const first = p_state->first;
	const char* const last = p_state->last;
	const char* p = p_state->p;
	const char* const p_save = p;

	assert( first );
	assert( last );
	assert( p >= first );

    if(p == last)
        return EOF;

	printf("%c (%d)\n", *p_save, *p_save);

	if(*p == '\x1e' || *p == '\x1f')
	{
		p_state->p = p + 1;
		p_loc->first_column = p_save - first + 1;
		p_loc->last_column =  p_save - first + 1;

		return *p;
	}

	while(p != last && *p != '\x1e' && *p != '\x1f')
		++p;

	p_state->p = p;
	size_t matchlen = p - p_save;

	p_loc->first_column = p_save - first + 1;
	p_loc->last_column = p - first;

	printf("%d:%d %zu\n", p_loc->first_column, p_loc->last_column, matchlen);

	if( !p_state->tokenize_header )
		return NON_SEPARATOR;

	if( !strncmp("E", p_save, matchlen) )
		return TOPIC_EVENT;
	if( !strncmp("R", p_save, matchlen) )
		return TOPIC_RECORD;
	if( !strncmp("S", p_save, matchlen) )
		return ACTION_SUBSCRIBE;
	if( !strncmp("A", p_save, matchlen) )
		return ACK;

	return TOPIC_UNKNOWN;
}


void yyerror(const YYLTYPE* p_loc, const struct state* p_state, const char* str)
{
	(void) p_state;
	(void) str;

	fprintf(
		stderr, "error %d bytes into the message\n",
		p_loc->first_column);
}


int main()
{
    const char INPUT[] = {
		'E', '\x1f', 'A', '\x1f', 'S', '\x1f','f', 'o', 'o', '\x1e', 0
	};

	const char* first = INPUT;
	const char* last = INPUT + strlen(INPUT);

	struct state state = { first, last, true, first, 0 };
    int ret = yyparse(&state);

    printf("ret = %d args=%zu\n", ret, state.num_data);
}
