/*
 * This file is part of PRO CFW.

 * PRO CFW is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * PRO CFW is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PRO CFW. If not, see <http://www.gnu.org/licenses/ .
 */

#ifndef _MISSINGFUNC_H_
#define _MISSINGFUNC_H_

/*
 * Missing Function Header (mostly for nidresolver.c)
 * Contains all reimplemented Function Prototypes that Sony removed in Updates.
 */

int setjmp(void);
int longjmp(void);
/*
int strcspn(char * str1, char * str2);
int strspn(char * str1, char * str2);
char * strncat(char * dst, const char * src, size_t n);
char * strtok(char * s, const char * delim);
*/
char * strtok_r(char * s, const char * delim, char ** last);
int strtol10(const char * str, int * res);
int strcasecmp(const char * a, const char * b);

#endif

