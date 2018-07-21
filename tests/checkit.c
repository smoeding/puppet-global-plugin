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

#include <stdio.h>
#ifdef STDC_HEADERS
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif

#ifdef HAVE_DIRENT_H
#include <dirent.h>
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

#include "parser.h"

void parser(struct parser_param *);


void callback(int a, const char *symbol, int lineno, const char *file, const char *line, void *f) {
  printf("%s: lineno=%d, file=%s, line=%s\n", symbol, lineno, file, line);
}


void check(char *file) {
  struct parser_param param;

  param.size  = sizeof(param);
  param.flags = PARSER_DEBUG;
  param.file  = file;
  param.put   = callback;
  param.arg   = NULL;

  parser(&param);
}


void recurse(const char *name) {
  struct dirent *entry;
  DIR *dir;

  if (!(dir = opendir(name))) return;
  if (!(entry = readdir(dir))) return;

  do {
    char path[1024];
    int len = snprintf(path, sizeof(path)-1, "%s/%s", name, entry->d_name);

    path[len] = 0;

    if (entry->d_type == DT_DIR) {
      if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        continue;

      recurse(path);
    }
    else {
      char *dot = strrchr(entry->d_name, '.');
      if (dot && !strcmp(dot, ".pp")) {
        check(path);
      }
    }
  } while ((entry = readdir(dir)));

  closedir(dir);
}


int main(int argc, char *argv[]) {
  struct stat sb;
  int i;

  for(i=1; i<argc; i++) {

    switch(stat(argv[i], &sb)) {
    case -1:
      perror(argv[i]);
      break;

    default:
      switch(sb.st_mode & S_IFMT) {

      case S_IFDIR:
        /* directory */
        recurse(argv[i]);
        break;

      case S_IFREG:
        /* regular file */
        check(argv[i]);
        break;

      default:
        /* otherwise do nothing */
        break;
      }
      break;
    }
  }

  return 0;
}
