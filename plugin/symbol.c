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
#include <ctype.h>
#endif
#include <stdio.h>
#include <errno.h>
#ifdef HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif

#include "errnum.h"
#include "file.h"
#include "stack.h"
#include "symbol.h"

#include "parser.h"

void yyrestart(FILE*);
int yyparse(void);


/* global reference to parser data for callbacks */
static struct parser_param *parser_callback;

/* builtin types that should not be added to the tags database */
static char *builtin_types[] = {
  "augeas", "computer", "cron", "exec", "file", "filebucket", "group",
  "host", "interface", "k5login", "macauthorization", "mailalias", "maillist",
  "mcx", "mount", "nagios_command", "nagios_contact", "nagios_contactgroup",
  "nagios_host", "nagios_hostdependency", "nagios_hostescalation",
  "nagios_hostextinfo", "nagios_hostgroup", "nagios_service",
  "nagios_servicedependency", "nagios_serviceescalation",
  "nagios_serviceextinfo", "nagios_servicegroup", "nagios_timeperiod",
  "notify", "package", "resources", "router", "schedule", "scheduled_task",
  "selboolean", "selmodule", "service", "ssh_authorized_key", "sshkey",
  "stage", "tidy", "user", "vlan", "yumrepo", "zfs", "zone", "zpool",
};


/*
 * Check for a Puppet builtin symbol
 *
 * @param symbol a pointer to the symbol name
 */
static int is_builtin_type(char *symbol) {
  size_t slen  = strlen(symbol);
  int    limit = sizeof(builtin_types) / sizeof(builtin_types[0]);
  int    i;

  for(i = 0; i < limit; i++) {
    size_t blen = strlen(builtin_types[i]);

    if ((slen == blen) && (strncmp(symbol, builtin_types[i], slen) == 0)) {
      return 1;
    }
  }

  return 0;
}


/*
 * Canonicalize symbol name
 * - remove single quotes
 * - remove namespace prefix
 *
 * @param symbol a pointer to the symbol name
 */
static char *canonicalize(char *symbol) {

  /* remove single quotes in string */
  if (*symbol == '\'') {
    char *tail = strchr(symbol, '\'');
    if (tail != NULL) {
      *tail = 0;
      symbol++;
    }
  }

  return (strncmp(symbol, "::", 2) == 0) ? (symbol + 2) : symbol;
}


/*
 * Symbol using lowercase letters
 *
 * @param symbol a pointer to the symbol name
 */
static char *lowercase(char *symbol) {
  char *ptr;

  for (ptr = symbol; *ptr; ptr++) {
    *ptr = tolower(*ptr);
  }

  return symbol;
}


/*
 * Symbol using Camel::Case components
 *
 * @param symbol a pointer to the symbol name
 */
static char *capitalize(char *symbol) {
  int   begin_of_component = 1;
  char *ptr;

  for (ptr = symbol; *ptr; ptr++) {
    switch(begin_of_component) {
    case 1:
      *ptr = toupper(*ptr);  /* uppercase the first character */

      if (*ptr != ':') begin_of_component = 0;
      break;

    case 0:
      *ptr = tolower(*ptr);  /* lowercase the remaining characters */

      if (*ptr == ':') begin_of_component = 1;
      break;
    }
  }

  return symbol;
}


/*
 * Store a defined type symbol
 *
 * @param symbol a pointer to the symbol name
 * @param lineno the line in the file
 */
void symdefine(char* symbol, int lineno) {
  char *sym;

  if (symbol) {
    sym = canonicalize(symbol);
    sym = lowercase(sym);

    if (!is_builtin_type(sym)) {
      parser_callback->put(PARSER_DEF, sym, lineno,
                           parser_callback->file, file_get(lineno),
                           parser_callback->arg);

      sym = capitalize(sym);

      parser_callback->put(PARSER_DEF, sym, lineno,
                           parser_callback->file, file_get(lineno),
                           parser_callback->arg);
    }

    free(symbol);
  }
}


/*
 * Store a class symbol
 *
 * @param symbol a pointer to the symbol name
 * @param lineno the line in the file
 */
void symclass(char* symbol, int lineno) {
  char *sym;

  if (symbol) {
    sym = canonicalize(symbol);
    sym = lowercase(sym);

    if (!is_builtin_type(sym)) {
      parser_callback->put(PARSER_DEF, sym, lineno,
                           parser_callback->file, file_get(lineno),
                           parser_callback->arg);
    }

    free(symbol);
  }
}


/*
 * Store a symbol reference
 *
 * @param symbol a pointer to the symbol name
 * @param lineno the line in the file
 */
void symref(char* symbol, int lineno) {
  char *sym;

  if (symbol) {
    sym = canonicalize(symbol);
    sym = lowercase(sym);

    if (!is_builtin_type(sym)) {
      parser_callback->put(PARSER_REF_SYM, sym, lineno,
                           parser_callback->file, file_get(lineno),
                           parser_callback->arg);
    }

    free(symbol);
  }
}


/*
 * Free storage for a symbol
 *
 * @param symbol a pointer to the symbol name
 */
void symfree(char* symbol) {
  if (symbol) free(symbol);
}


/*
 * Main parser entry function called from the global executable
 *
 * @param param defined in parser.h
 */
void parser(struct parser_param *param) {
  extern int  yylineno;
  FILE       *file = fopen(param->file, "r");

  if (file) {
    int err = file_read(file);

    parser_callback = param;

    switch (err) {
    case NO_ERROR:
      yylineno = 1;
      yyrestart(file);

      yyparse();

      file_finish();
      fclose(file);
      break;

    case ERROR_NO_MEM:
      param->message("%s: out of memory", param->file);
      break;

    case ERROR_SHORT_READ:
      param->message("%s: short read", param->file);
      break;
    }
  }
  else {
    param->message("%s: can't open file; errno=%d", param->file, errno);
  }
}


/*
 * Error handler for the parser
 *
 * @param message the error message
 * @param lineno  the line where the error occured (yylineno)
 * @param text    the parsed text when the error occured (yytext)
 */
void report_error(char *message, int lineno, char *text) {
  parser_callback->message("%s: %s at line %d near '%s'",
                           parser_callback->file, message, lineno, text);
}
