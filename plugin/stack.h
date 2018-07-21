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


#ifndef SYMBOLSTACK_H_DONE
#define SYMBOLSTACK_H_DONE


typedef struct stack *Stack;

char *stack_top(Stack stack);
Stack stack_pop(Stack stack);
Stack stack_push(Stack stack, char *symbol);
Stack stack_init();


/*
 * Usage:
 *
 * Stack s = stack_init();
 *
 * s = stack_push(s, "foo");
 * s = stack_push(s, "bar");
 * s = stack_push(s, "baz");
 *
 * while (s) {
 *   printf("%s\n", stack_top(s));
 *   s = stack_pop(s);
 * }
 *
 */

#endif
