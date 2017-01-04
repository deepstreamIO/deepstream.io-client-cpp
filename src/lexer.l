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
%option fast


%{
#include <parser.h>
%}

/* message part separator */
MPS [\x1f]
/* message separator */
MS  [\x1e]
/* non-separator */
NS  [^\x1e^\x1f]


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
