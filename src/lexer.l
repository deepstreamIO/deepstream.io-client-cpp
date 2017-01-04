%option noyywrap
%option reentrant
%option extra-type="struct deepstream_parser_state*"
%option fast


%{
#include "parser.h"
%}

/* message part separator */
MPS [\x1f]
/* message separator */
MS  [\x1e]
/* non-separator */
NS  [^\x1e^\x1f]

/*{NS}.*{MS}            { return DS_PARSE(TOKEN_UNKNOWN); };*/

%%
^A{MPS}A                { return DS_PARSE(TOKEN_A_A); }
^A{MPS}E{MPS}INVALID_AUTH_DATA { return DS_PARSE(TOKEN_A_E_IAD); };
^A{MPS}E{MPS}TOO_MANY_AUTH_ATTEMPTS { return DS_PARSE(TOKEN_A_E_TMAA); };
^A{MPS}REQ              { return DS_PARSE(TOKEN_A_REQ); };
^E{MPS}A{MPS}L          { return DS_PARSE(TOKEN_E_A_L); };
^E{MPS}A{MPS}S          { return DS_PARSE(TOKEN_E_A_S); };
^E{MPS}L                { return DS_PARSE(TOKEN_E_L); };
^E{MPS}S                { return DS_PARSE(TOKEN_E_S); };
^E{MPS}US               { return DS_PARSE(TOKEN_E_US); };
{MPS}{NS}+              { return DS_PARSE(TOKEN_PAYLOAD); };
{MS}                    { return DS_PARSE(TOKEN_RECORD_SEPARATOR); };
<<EOF>>                 { return DS_PARSE(TOKEN_EOF); }

%%
