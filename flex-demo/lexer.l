%option noyywrap
%option reentrant
%option extra-type="struct deepstream_parser_state*"


%{
#include "parser.h"
%}

/* message part separator */
MPS [\x1f]
/* message separator */
MS  [\x1e]
/* non-separator */
NS  [^\x1e^\x1f]

/*{NS}.*{MS}                { return DS_PARSE(TOKEN_UNKNOWN); };*/

%%
^E{MPS}A{MPS}S            { return DS_PARSE(TOKEN_E_A_S); };
^E{MPS}L                  { return DS_PARSE(TOKEN_E_L); };
{MPS}{NS}+                { return DS_PARSE(TOKEN_PAYLOAD); };
{MS}                      { return DS_PARSE(TOKEN_RECORD_SEPARATOR); };
<<EOF>>                   { return DS_PARSE(TOKEN_EOF); }

%%
