%code top {
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


#include "symbol.h"

int yylex(void);
void yyerror (char const *);

}

%locations
%define api.value.type {char*}

%nonassoc UNARY_OPERATOR UNARY

%left OPERATOR
%left '.'

%token BEFORE
%token CASE
%token CLASS
%token CONTAIN
%token DEFINE
%token ELSE
%token ELSIF
%token HASHROCKET
%token HEREDOC
%token IF
%token INCLUDE
%token INHERITS
%token ITERATOR
%token LOGGING
%token NOTIFY
%token NUMBER
%token REGEXP
%token REQUIRE
%token RESNAME
%token RESREF
%token SPACESHIP
%token STRING
%token SUBSCRIBE
%token UNLESS
%token VARNAME
%token DATATYPE

%%

input:          /* empty */
        |       input class
        |       input define
        |       input assignment
        |       input resource
        |       input function_stmt
        |       input conditional_stmt
        |       input simple_expression
                ;

assignment:     VARNAME '=' expression
                ;

define:         DEFINE RESNAME block                       { symdefine($2, @2.first_line); }
        |       DEFINE RESNAME '(' argument_list ')' block { symdefine($2, @2.first_line); }
                ;

class:          CLASS RESNAME inherits block                       { symclass($2, @2.first_line); }
        |       CLASS RESNAME '(' argument_list ')' inherits block { symclass($2, @2.first_line); }
                ;

inherits:       /* empty */
        |       INHERITS RESNAME { symref($2, @2.first_line); }
                ;

block:          '{' code '}'
                ;

code:           /* empty */
        |       code assignment
        |       code resource
        |       code function_stmt
        |       code conditional_stmt
        |       code simple_expression
        |       code lambda
                ;

resource:       RESNAME '{' resource_decl_list '}'         { symref($1, @1.first_line); }
        |       '@' RESNAME '{' resource_decl_list '}'     { symref($2, @2.first_line); }
        |       '@' '@' RESNAME '{' resource_decl_list '}' { symref($3, @3.first_line); }
        |       CLASS '{' resource_decl_list '}'           /* FIXME: stack */
        |       NOTIFY '{' resource_decl_list '}'
        |       RESREF array                                                 { symref($1, @1.first_line); }
        |       RESREF '{' parameter_list '}'                                { symref($1, @1.first_line); }
        |       RESREF array '{' parameter_list '}'                          { symref($1, @1.first_line); }
        |       RESREF SPACESHIP SPACESHIP                                   { symref($1, @1.first_line); }
        |       RESREF SPACESHIP SPACESHIP '{' parameter_list '}'            { symref($1, @1.first_line); }
        |       RESREF SPACESHIP expression SPACESHIP                        { symref($1, @1.first_line); }
        |       RESREF SPACESHIP expression SPACESHIP '{' parameter_list '}' { symref($1, @1.first_line); }
                ;

resource_decl_list:
                resource_decl
        |       resource_decl ';'
        |       resource_decl ';' resource_decl_list
                ;

resource_decl:  STRING ':' parameter_list
        |       VARNAME element_access ':' parameter_list
        |       array ':'  parameter_list
                ;

function_stmt:  INCLUDE STRING  /* FIXME */
        |       INCLUDE RESNAME { symref($2, @2.first_line); }
        |       INCLUDE VARNAME
        |       REQUIRE STRING  /* FIXME */
        |       REQUIRE RESNAME { symref($2, @2.first_line); }
        |       REQUIRE VARNAME
        |       CONTAIN STRING  /* FIXME */
        |       CONTAIN RESNAME { symref($2, @2.first_line); }
        |       CONTAIN VARNAME
        |       LOGGING expression
        |       LOGGING '(' function_param_list ')'
                ;

function_call:  RESNAME '(' function_param_list ')' { symfree($1); }
                ;

function_param_list:
                /* empty */
        |       expression
        |       expression ',' function_param_list
                ;

conditional_stmt:
                CASE expression '{' case_block '}'
        |       IF expression block elsif_stmt
        |       IF expression block elsif_stmt ELSE block
        |       UNLESS expression block
                ;

elsif_stmt:     /* empty */
        |       ELSIF expression block elsif_stmt
                ;

case_block:     case_expression ':' block
        |       case_block case_expression ':' block
                ;

case_expression:STRING
        |       NUMBER
        |       REGEXP
        |       RESNAME                     { symfree($1); }
        |       STRING  ',' case_expression
        |       NUMBER  ',' case_expression
        |       REGEXP  ',' case_expression
        |       RESNAME ',' case_expression { symfree($1); }
                ;

parameter_list: /* empty */
        |       parameter
        |       parameter ',' parameter_list
                ;

parameter:      VARNAME   HASHROCKET expression
        |       RESNAME   HASHROCKET expression { symfree($1); }
        |       RESREF    HASHROCKET expression { symfree($1); }
        |       STRING    HASHROCKET expression
        |       NUMBER    HASHROCKET expression
        |       REGEXP    HASHROCKET expression
        |       UNLESS    HASHROCKET expression
        |       REQUIRE   HASHROCKET expression
        |       NOTIFY    HASHROCKET expression
        |       BEFORE    HASHROCKET expression
        |       SUBSCRIBE HASHROCKET expression
                ;

argument_list:  /* empty */
        |       argument
        |       argument ',' argument_list
                ;

argument:       VARNAME
        |       VARNAME '=' expression
        |       DATATYPE VARNAME
        |       DATATYPE VARNAME '=' expression
                ;

expression_list: /* empty */
        |       expression
        |       expression ',' expression_list
                ;

element_access: /* empty */
        |       element_access '[' expression ']'
                ;

simple_expression:
                '(' expression ')'
        |       VARNAME element_access
        |       function_call
        |       STRING
        |       NUMBER
        |       array
        |       hash
        |       selector
                ;

expression:     simple_expression
        |       UNARY_OPERATOR expression %prec UNARY
        |       expression OPERATOR expression
        |       expression OPERATOR REGEXP
        |       RESNAME      { symfree($1); }
        |       RESREF       { symfree($1); }
        |       RESREF array { symfree($1); }
        |       lambda
        |       HEREDOC
                ;

selector:       expression '?' '{' parameter_list '}'
                ;

variable_list:  VARNAME
        |       VARNAME ',' variable_list
                ;

lambda:         ITERATOR '(' function_param_list ')' '|' variable_list '|' block
        |       expression '.' ITERATOR '|' variable_list '|' block
                ;

array:          '[' expression_list ']'
                ;

hash:           '{' hash_item_list '}'
                ;

hash_item_list: /* empty */
        |       hash_item
        |       hash_item ',' hash_item_list
                ;

hash_item:      STRING    HASHROCKET expression
        |       VARNAME   HASHROCKET expression
        |       RESNAME   HASHROCKET expression { symfree($1); }
        |       RESREF    HASHROCKET expression { symfree($1); }
        |       REQUIRE   HASHROCKET expression
        |       BEFORE    HASHROCKET expression
        |       SUBSCRIBE HASHROCKET expression
        |       NOTIFY    HASHROCKET expression
                ;
