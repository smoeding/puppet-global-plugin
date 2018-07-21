/*
 * Copyright (c) 2016 Stefan Möding <stm@kill-9.net>
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

#include "errnum.h"
#include "file.h"


static char* buffer = NULL;
static Line* line   = NULL;


/*
 * Read a file and keep an array of all lines
 */
int file_read(FILE* file) {
  size_t  fsize, rsize;
  char   *bol, *eot, *ptr;
  int     lineno, pos;

  /* jump to end of file and get that position */
  (void)fseek(file, 0L, SEEK_END);
  fsize = ftell(file);
  rewind(file);

  if (fsize == 0) return ERROR_ZERO_FILE_SIZE;

  /* allocate buffer for file and an additional NUL char */
  buffer = (char*)malloc(fsize + 1);

  if (buffer == NULL) return ERROR_NO_MEM;

  /* read file into buffer */
  rsize = fread(buffer, 1, fsize, file);
  rewind(file);

  if (rsize < fsize) return ERROR_SHORT_READ;

  /* initial position for beginning of first line */
  bol    = buffer;
  lineno = 1;

  /* NUL terminated end of text in buffer */
  eot  = &buffer[fsize];
  *eot = 0;

  /* count number of lines in the buffered file */
  for(ptr=buffer; ptr<eot; ptr++) {
    if (*ptr == '\n') lineno++;
  }

  /* allocate array to hold references to each line */
  line = (Line*)calloc(lineno, sizeof(Line));

  if (line == NULL) return ERROR_NO_MEM;

  /* array position 0 is for line 1 */
  pos = 0;

  /* build array with line references */
  for(ptr=buffer; ptr<eot; ptr++) {
    if (*ptr == '\n') {
      *ptr = 0;           /* set string terminator for line */
      line[pos] = bol;    /* store address of line in array */

      bol = ptr + 1;      /* following line starts at next char */
      pos++;
    }
  }

  return NO_ERROR;
}


/*
 * cleanup file buffer and array of lines
 */
void file_finish() {
  if (buffer) {
    free(buffer);
    buffer = NULL;
  }

  if (line) {
    free(line);
    line = NULL;
  }
}


/*
 * return a pointer to the line with a given lineno
 */
Line file_get(int lineno) {
  /* first line is stored as line[0], so decrement parameter */
  return line ? line[lineno-1] : NULL;
}
