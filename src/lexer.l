/*
 * Copyright 2016-2017 deepstreamHub GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
%option noyywrap
%option reentrant
%option extra-type="struct deepstream_parser_state*"
%option 8bit
%option fast


%{
#include <deepstream/parser.h>
%}

/* message part separator */
MPS [\x1f]
/* message separator */
MS  [\x1e]
/* non-separator */
NS  [^\x1e\x1f]


/*
 * From the documentation:
 *  For historical reasons, start conditions do not have their own name-space
 *  within the generated scanner. The start condition names are unmodified in
 *  the generated scanner and generated header.
 */
%x yypayload yyerror

/*
 * A proper message will cause the lexer to return the following tokens:
 * - a header token, e.g., TOKEN_A_A,
 * - TOKEN_PAYLOAD zero or more times,
 * - TOKEN_MESSAGE_SEPARATOR.
 * If an unknown token is encountered, the lexer discards input until it
 * encounters a message separator. The question is now if for every unknown
 * token, the lexer should return one token (TOKEN_UNKNOWN) or two tokens
 * (TOKEN_UNKNOWN followed by TOKEN_MESSAGE_SEPARATOR). This lexer returns only
 * one token because the invalid message may consist only of the message
 * separator; with two tokens as return value, one of the tokens must correspond
 * to a match of length zero and the scanner must return two tokens although it
 * consumed only a single character.
 */

%%

<INITIAL>{
	A{MPS}A        { BEGIN(yypayload); return DS_PARSE(TOKEN_A_A); }
	A{MPS}E{MPS}INVALID_AUTH_DATA { BEGIN(yypayload); return DS_PARSE(TOKEN_A_E_IAD); };
	A{MPS}E{MPS}INVALID_AUTH_MSG { BEGIN(yypayload); return DS_PARSE(TOKEN_A_E_IAM); };
	A{MPS}E{MPS}TOO_MANY_AUTH_ATTEMPTS { BEGIN(yypayload); return DS_PARSE(TOKEN_A_E_TMAA); };
	A{MPS}REQ      { BEGIN(yypayload); return DS_PARSE(TOKEN_A_REQ); };

	C{MPS}A        { BEGIN(yypayload); return DS_PARSE(TOKEN_C_A); }
	C{MPS}CH       { BEGIN(yypayload); return DS_PARSE(TOKEN_C_CH); }
	C{MPS}CHR      { BEGIN(yypayload); return DS_PARSE(TOKEN_C_CHR); }
	C{MPS}PI       { BEGIN(yypayload); return DS_PARSE(TOKEN_C_PI); }
	C{MPS}PO       { BEGIN(yypayload); return DS_PARSE(TOKEN_C_PO); }
	C{MPS}RED      { BEGIN(yypayload); return DS_PARSE(TOKEN_C_RED); }
	C{MPS}REJ      { BEGIN(yypayload); return DS_PARSE(TOKEN_C_REJ); }

	E{MPS}A{MPS}L  { BEGIN(yypayload); return DS_PARSE(TOKEN_E_A_L); };
	E{MPS}A{MPS}S  { BEGIN(yypayload); return DS_PARSE(TOKEN_E_A_S); };
	E{MPS}A{MPS}US { BEGIN(yypayload); return DS_PARSE(TOKEN_E_A_US); };
	E{MPS}EVT      { BEGIN(yypayload); return DS_PARSE(TOKEN_E_EVT); };
	E{MPS}L        { BEGIN(yypayload); return DS_PARSE(TOKEN_E_L); };
	E{MPS}S        { BEGIN(yypayload); return DS_PARSE(TOKEN_E_S); };
	E{MPS}SP       { BEGIN(yypayload); return DS_PARSE(TOKEN_E_SP); };
	E{MPS}SR       { BEGIN(yypayload); return DS_PARSE(TOKEN_E_SR); };
	E{MPS}US       { BEGIN(yypayload); return DS_PARSE(TOKEN_E_US); };

	U{MPS}A{MPS}S  { BEGIN(yypayload); return DS_PARSE(TOKEN_U_A_S); };
	U{MPS}A{MPS}US { BEGIN(yypayload); return DS_PARSE(TOKEN_U_A_US); };
	U{MPS}PNJ      { BEGIN(yypayload); return DS_PARSE(TOKEN_U_PNJ); };
	U{MPS}PNL      { BEGIN(yypayload); return DS_PARSE(TOKEN_U_PNL); };
	U{MPS}Q        { BEGIN(yypayload); return DS_PARSE(TOKEN_U_Q); };
	U{MPS}Q{MPS}Q  { BEGIN(yypayload); return DS_PARSE(TOKEN_U_Q_Q); };
	U{MPS}S{MPS}S  { BEGIN(yypayload); return DS_PARSE(TOKEN_U_S); };
	U{MPS}US{MPS}US { BEGIN(yypayload); return DS_PARSE(TOKEN_U_US); };

	{MS}           { return DS_PARSE(TOKEN_UNKNOWN); };
	.              { BEGIN(yyerror); yymore(); };
	\n             { BEGIN(yyerror); yymore(); };
}


<yypayload>{
	{MPS}{NS}+   { return DS_PARSE(TOKEN_PAYLOAD); };
	{MS}         { BEGIN(INITIAL); return DS_PARSE(TOKEN_MESSAGE_SEPARATOR); };

	.            { BEGIN(yyerror); yymore(); }
}

<yyerror>{
	[^\x1e]*{MS}*  { BEGIN(INITIAL); return DS_PARSE(TOKEN_UNKNOWN); };
	<<EOF>>        { BEGIN(INITIAL); yyless(1); return DS_PARSE(TOKEN_UNKNOWN); };
}

<<EOF>>            { (void)DS_PARSE(TOKEN_EOF); yyterminate(); };

%%
