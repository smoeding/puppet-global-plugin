/**************************************************************************
 *
 * Copyright (c) 2018 Stefan Möding <stm@kill-9.net>
 *
 * This file is NOT part of GNU GLOBAL.
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


#if HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef STDC_HEADERS
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <errno.h>
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_MMAP
#include <sys/mman.h>
#endif

#include "parser.h"


/* state of the tokenizer */
typedef struct {
  char *pos;           /* current position in file while reading */
  char *bot;           /* begin of token */
  char *end;           /* end of file */
  int state;           /* state of the tokenizer */
  char *token;         /* current token as C string (malloc) */
  char *bol;           /* beginning of line */
  char *line;          /* current line as C string (malloc) */
  int current_line;    /* current line number */
  int current_column;  /* current column number */
  int token_lineno;    /* line number of current token */
} Tokenizer;

enum TOKENIZER_STATE { BEGIN, COMMENT, TOKEN };


/*
 * Create a new Tokenizer
 *
 * @param buffer pointer to the memory buffer where processing should start
 * @param size the size of the memory buffer in bytes
 *
 * @return pointer to a Tokenizer struct
 */
Tokenizer *alloc_tokenizer(char *buffer, off_t size) {
  Tokenizer *tokenizer = malloc(sizeof(Tokenizer));

  if (tokenizer) {
    tokenizer->state          = BEGIN;
    tokenizer->current_line   = 1;
    tokenizer->current_column = 0;
    tokenizer->pos            = buffer;
    tokenizer->bot            = buffer;
    tokenizer->end            = buffer + size;
    tokenizer->token          = NULL;
    tokenizer->bol            = buffer;
    tokenizer->line           = NULL;
  }

  return tokenizer;
}


/*
 * Deallocate memory for a Tokenizer
 *
 * @param tokenizer the reference returned by alloc_tokenizer
 */
static void free_tokenizer(Tokenizer *tokenizer) {
  if (tokenizer) {

    if (tokenizer->token) free(tokenizer->token);
    if (tokenizer->line) free(tokenizer->line);

    free(tokenizer);
  }
}


/*
 * Return the line number where the current token is in the file
 *
 * @param tokenizer the reference returned by alloc_tokenizer
 *
 * @return the line number of the token
 */
static int token_lineno(Tokenizer *tokenizer) {return tokenizer->token_lineno;}


/*
 * Return the complete line where the token is located in the file
 *
 * @param tokenizer a pointer to a Tokenizer object
 *
 * @return a NUL terminated string of the complete line
 */
static char *token_line(Tokenizer *tokenizer) {
  char *pos = tokenizer->bol;
  int size;

  // Deallocate memory for previous line
  if (tokenizer->line) {
    free(tokenizer->line);
    tokenizer->line = NULL;
  }

  if (tokenizer->bol) {

    // Look for end of line
    while (*pos != '\n') pos++;

    size = pos - tokenizer->bol;

    tokenizer->line = malloc(size + 1);

    if (tokenizer->line) {
      memcpy(tokenizer->line, tokenizer->bol, size);
      tokenizer->line[size] = '\000';
    }
  }

  return tokenizer->line;
}


/*
 * Return the next token from the tokenizer
 *
 * @param tokenizer a pointer to a Tokenizer object
 *
 * @return a NUL terminated string of the token
 */
static char *get_token(Tokenizer *tokenizer) {
  int size;

  // Deallocate memory for previous token
  if (tokenizer->token) {
    free(tokenizer->token);
    tokenizer->token = NULL;
  }

  while(tokenizer->pos < tokenizer->end) {
    char looking_at = *tokenizer->pos;

    // Update line/column counters
    if (looking_at == '\n') {
      tokenizer->current_line++;
      tokenizer->current_column = 0;
      tokenizer->bol = (tokenizer->pos) + 1;
    }
    else {
      tokenizer->current_column++;
    }

    switch(tokenizer->state) {
    case BEGIN:

      switch(looking_at) {
      case '\n':
      case '\t':
      case '\f':
      case ' ':
        // Ignore whitespace and newlines
        break;

      case '#':
        // Start of a comment
        tokenizer->state = COMMENT;
        break;

      case 'a' ... 'z':
      case 'A' ... 'Z':
      case '0' ... '9':
      case '_':
      case ':':
        // No whitespace, no comment
        tokenizer->state = TOKEN;

        tokenizer->bot          = tokenizer->pos;
        tokenizer->token_lineno = tokenizer->current_line;

        break;

      default:
        // Something else
        return NULL;
      }
      break;

    case COMMENT:
      // Continue scanning until end-of-line marker
      if (looking_at == '\n') tokenizer->state = BEGIN;
      break;

    case TOKEN:
      switch(looking_at) {
      case 'a' ... 'z':
      case 'A' ... 'Z':
      case '0' ... '9':
      case '_':
      case ':':
        // Read over valid token characters till we find the end
        break;

      default:
        // End of token
        size = tokenizer->pos - tokenizer->bot;

        // Allocate memory to store a copy of the token as C string
        tokenizer->token = malloc(size+1);

        if (tokenizer->token) {
          memcpy(tokenizer->token, tokenizer->bot, size);
          tokenizer->token[size] = '\000';
        }

        tokenizer->state = BEGIN;

        break;
      }

      break;
    }

    // Continue with the next char in the file
    tokenizer->pos++;

    if (tokenizer->token) break;
  }

  return tokenizer->token;
}


/*
 * Check if the token is a valid manifest start keyword
 *
 * @param token a string to check for a valid keyword
 */
static int is_manifest(const char *token) {

  if (strcmp(token, "class") == 0) return 1;
  if (strcmp(token, "define") == 0) return 1;

  return 0;
}


/*
 * Main parser entry function called from the global executable
 *
 * @param param defined in parser.h
 */
void parser(struct parser_param *parser) {
  struct stat  sb;
  Tokenizer   *tokenizer;
  char        *buffer;
  char        *token;
  int          fd;

  fd = open(parser->file, O_RDONLY);

  if (fd >= 0) {
    if (fstat(fd, &sb) == 0) {
      if (S_ISREG(sb.st_mode)) {
        if (sb.st_size > 0) {

          buffer = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

          if (buffer != MAP_FAILED) {
            tokenizer = alloc_tokenizer(buffer, sb.st_size);

            if (tokenizer) {

              // Get first token (should be class, define, ...)
              token = get_token(tokenizer);

              if (token && is_manifest(token)) {
                // Get next token
                token = get_token(tokenizer);

                if (token) {
                  // Skip initial '::'
                  while(*token == ':') { token++; }

                  // Put location of current token into the global database
                  parser->put(PARSER_DEF,
                              token,
                              token_lineno(tokenizer),
                              parser->file,
                              token_line(tokenizer),
                              parser->arg);
                }
              }

              free_tokenizer(tokenizer);
            }
            else {
              parser->message("%s: failed to allocate tokenizer", parser->file);
            }

            if (munmap(buffer, sb.st_size) < 0) {
              parser->message("%s: failed to munmap file", parser->file);
            }
          }
          else {
            parser->message("%s: failed to mmap file", parser->file);
          }
        }
      }
    }
    else {
      parser->message("%s: can't stat file; errno=%d", parser->file, errno);
    }

    close(fd);
  }
  else {
    parser->message("%s: can't open file; errno=%d", parser->file, errno);
  }
}
