%{
/*
 * Copyright (c) 2016, 2018 Stefan Möding <stm@kill-9.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 **************************************************************************/


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef STDC_HEADERS
#include <stdlib.h>
#endif
#include <stdio.h>
#ifdef HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif

#include "pp_parser.h"

#define YY_USER_ACTION yylloc.first_line = yylloc.last_line = yylineno;

void heredoc();
void comment();
void report_error(char *message, int lineno, char *text);

void yyerror(char *s) {
  report_error(s, yylineno, yytext);
}

%}

%option nodefault yylineno noyywrap nounput never-interactive

%%

"/*"                   { comment(); }

\n                     { }

"+>" |
"=>"                   return HASHROCKET;

"->" |
"~>"                   /* ignore resource chains */

"<|"  |
"<<|" |
"|>"  |
"|>>"                  return SPACESHIP;

"<<"  |
">>"  |
"!="  |
"<"   |
"<="  |
"=="  |
"=~"  |
"!~"  |
"in"  |
">"   |
">="  |
"+"   |
"-"   |
"*"   |
"/"   |
"%"   |
"and" |
"or"                   return OPERATOR;

"!"                    return UNARY_OPERATOR;

"|" |
"[" |
"]" |
"(" |
")" |
"{" |
"}" |
"," |
":" |
";" |
"." |
"?" |
"@" |
"="                    return yytext[0];

"debug"   |
"err"     |
"fail"    |
"info"    |
"notice"  |
"warning"              return LOGGING;

"each"   |
"filter" |
"map"    |
"reduce" |
"slice"  |
"with"                 return ITERATOR;

"before"               return BEFORE;
"notify"               return NOTIFY;
"require"              return REQUIRE;
"subscribe"            return SUBSCRIBE;

"class"                return CLASS;
"define"               return DEFINE;

"case"                 return CASE;
"contain"              return CONTAIN;
"else"                 return ELSE;
"elsif"                return ELSIF;
"if"                   return IF;
"include"              return INCLUDE;
"inherits"             return INHERITS;
"unless"               return UNLESS;


@\([^\n:/\(\)]+[/nrtsuL$]*\) |
@\(\"[^\n:/\(\)]+[/nrtsuL$]*?\"\) { heredoc(); return HEREDOC; }

-?[0-9]+(\.[0-9]*)?([eE][0-9]+)? return NUMBER; /* decimal & octal numbers */
0x[0-9a-fA-F]+                   return NUMBER; /* hexadecimal numbers */

[A-Z][A-Za-z:]* |
[A-Z][A-Za-z:]*\[.+\]            return DATATYPE;

$([a-z0-9_][a-zA-Z0-9_]*)?(::[a-z_][a-zA-Z0-9_]*)* return VARNAME;

([a-z][a-z0-9_]*)?(::[a-z][a-z0-9_]*)*             { yylval = strdup(yytext);
                                                     return RESNAME;
                                                   }

([A-Z][a-z0-9_]*)?(::[A-Z][a-z0-9_]*)*             { yylval = strdup(yytext);
                                                     return RESREF;
                                                   }

\"(\\.|[^\\"])*\"          return STRING;
\'(\\.|[^\\'])*\'          return STRING;
\/(\\.|[^\n\\/])*\/        { if (yytext[1] == '*') {
                               yyless(2);
                               comment();
                             } else {
                               return REGEXP;
                             }
                           }

#[^\n]*                /* ignore comments */
[ \t]+                 /* ignore whitespace */

.                      { yyerror("Illegal character"); }

%%

/*
 * Skip over a Puppet heredoc
 */
void heredoc() {
  char *buffer, *token, *marker;
  int state = 0, pos = 0, done = 0;
  int c;

  /* allocate buffer for matching text */
  buffer = strdup(yytext);
  if (buffer == NULL) yyerror("Out of memory");

  /* skip initial @ and ( characters */
  token = buffer + 2;

  /* overwrite ) marker at the end */
  marker = strchr(token, ')');
  if (marker != NULL) *marker = 0;

  if (*token == '"') {
    token++;

    /* also overwrite trailing quote */
    marker = strchr(token, '"');
    if (marker != NULL) *marker = 0;
  }

  /* overwrite escape switch marker / */
  marker = strchr(token, '/');
  if (marker != NULL) *marker = 0;

  while (((done == 0) && (c = input()) != EOF)) {
    switch (state) {
    case 0:
      /* read until end of line */
      if (c == '\n') state = 1;
      break;

    case 1:
      /* Does the line start with the | marker? */
      switch (c) {
      case ' ':
      case '\t':
        /* ignore whitespace */
        state = 1;
        break;

      case '|':
        /* | marker found; next state */
        state = 2;
        break;

      default:
        /* other character, continue reading the line */
        state = 0;
        break;
      }
      break;

    case 2:
      /* we have a line starting with the | marker */
      switch (c) {
      case ' ':
      case '-':
        /* ignore whitespace or - */
        break;

      default:
        if (token[pos] == c) {
          /* first character matches, so got to next state */
          state = 3;
          pos++;
        }
        else {
          /* no match of first character, so read rest of line and continue */
          state = 0;
          pos = 0;
        }
        break;
      }
      break;

    case 3:
      if (token[pos] == 0) {
        switch (c) {
        case '\n':
          done = 1;
          break;

        case ' ':
        case '\t':
          break;

        default:
          state = 0;
          break;
        }
      }

      if (token[pos] == c) {
        pos++;
      }
      else {
        pos = 0;
        state = 0;
      }
      break;
    }
  }

  free(buffer);
}

/*
 * Skip over a star comment
 */
void comment() {
  char curr, prev = ' ';

  while ((curr = input()) != EOF) {
    if ((curr == '/') && (prev == '*')) return;
    prev = curr;
  }
}
