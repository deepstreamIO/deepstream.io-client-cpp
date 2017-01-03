%option noyywrap
%option reentrant




%%
^E[\x1f]A[\x1f]S                printf("Event Ack Subscribe\n");
^E[\x1f]L                       printf("Event Listen\n");
[\x1e]$                         printf("No payload\n");
([\x1f][^\x1e^\x1f]+)+[\x1e]$   printf("Payload\n");
\n


%%
#include <string.h>

#include <assert.h>

#define TOKEN_EVENT_ACK_SUBSCRIBE
#define TOKEN_HAS_PAYLOAD
#define NO_PAYLOAD

// control memory consumption with yyalloc, yyrealloc, yyfree
int main()
{
    //char string[] = "E|A|S+\n";
    //char string[] = "E|A|S|foo|bar|baz+\n";
    char string[] = "E|L|name+\n";
	size_t size = strlen(string);

	for(size_t i = 0; i < size; ++i)
	{
		if(string[i] == '+') string[i] = '\x1e';
		if(string[i] == '|') string[i] = '\x1f';
	}


	yyscan_t scanner;
	yylex_init(&scanner);

	// yy_scan_bytes() copies the string!
	// consider fmemopen() as an alternative
	YY_BUFFER_STATE buffer = yy_scan_bytes(string, size, scanner);
	yy_switch_to_buffer(buffer, scanner);

	int ret = yylex(scanner);
	assert( ret == 0 );

	yy_delete_buffer(buffer, scanner);
	yylex_destroy(scanner);
}
