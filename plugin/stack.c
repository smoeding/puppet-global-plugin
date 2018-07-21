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

#include "stack.h"


struct stack {
  char  *data;
  Stack  next;
};


/*
 * Return element on top of stack or NULL of stack is empty
 */
char *stack_top(Stack stack) {
  return stack ? stack->data : NULL;
}


/*
 * Pop an element from stack
 */
Stack stack_pop(Stack stack) {
  Stack next = NULL;

  if (stack) {
    next = stack->next;
    free(stack);
  }

  return next;
}


/*
 * Push a new element to the stack
 */
Stack stack_push(Stack stack, char *symbol) {

  /* allocate new entry */
  Stack new = (Stack)malloc(sizeof(struct stack));

  if (new) {
    new->data = symbol;
    new->next = stack;  /* chain up current stack */
  }

  return new;
}


/*
 * initialize an empty stack
 */
Stack stack_init() {
  return NULL;
}
