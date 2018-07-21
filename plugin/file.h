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


#ifndef LINES_H_DONE
#define LINES_H_DONE

typedef char* Line;

enum {
  LINES_OK,
  LINES_ENOMEM,
  LINES_SHORT_READ,
};

int file_read(FILE* file);
void file_finish();

Line file_get(int lineno);

#endif
